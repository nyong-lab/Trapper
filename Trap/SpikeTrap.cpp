// Fill out your copyright notice in the Description page of Project Settings.

#include "Trap/SpikeTrap.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/TrapperPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "TrapperProject.h"
#include "Monster/BaseMonster.h"
#include "Ping/MarkerComponent/MapMarkerComponent.h"
#include "Components/AudioComponent.h"


ASpikeTrap::ASpikeTrap()
{
	MapMarkerwpqkfwpqkf->TrapType = EMapTrapType::SpikeAlive;
	PrimaryActorTick.bCanEverTick = true;

	SpikeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpikeMesh"));
	SpikeMesh->SetupAttachment(Mesh);

	ReAttackTime = 2.f;
	SpikeUpAndPause = 0.5f;
	Damage = 20.f;
	bOverlappingTrigger = false;
	bIsSpikeTimeline = false;
	bIsSpikeUp = false;

	Tags.Add(FName("SpikeTrap"));
}

ASpikeTrap::~ASpikeTrap()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpikeDownTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(ReAttackTimerHandle);
	}
}

void ASpikeTrap::BeginPlay()
{
	Super::BeginPlay();

	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ASpikeTrap::OnOverlapBeginCharacter);
	Trigger->OnComponentEndOverlap.AddDynamic(this, &ASpikeTrap::OnOverlapEndCharacter);
	SpikeMesh->OnComponentBeginOverlap.AddDynamic(this, &ASpikeTrap::OnOverlapBeginCharacterHit);

	InitialSpikeLocation = SpikeMesh->GetRelativeLocation();

	if (SpikeRiseCurve)
	{
		//UE_LOG(LogTrap, Warning, TEXT("SpikeRiseCurve"));

		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindDynamic(this, &ASpikeTrap::HandleSpikeProgress);
		SpikeTimeline.AddInterpFloat(SpikeRiseCurve, TimelineProgress);
		SpikeTimeline.SetTimelineLength(0.2f);

		FOnTimelineEvent TimelineFinished;
		TimelineFinished.BindDynamic(this, &ASpikeTrap::OnSpikeTimelineFinished);
		SpikeTimeline.SetTimelineFinishedFunc(TimelineFinished);

		SpikeTimeline.SetLooping(false);
	}

	if (HasAuthority())
	{
		if (IsValid(Owner))
		{
			ATrapperPlayer* Player = Cast<ATrapperPlayer>(Owner);
			if (IsValid(Player))
			{
				OwnerController = Player->GetController();
			}
		}
	}
}

void ASpikeTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void ASpikeTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsSpikeTimeline)
	{
		SpikeTimeline.TickTimeline(DeltaTime);
	}
}

void ASpikeTrap::Demolish()
{
	FTransform Transform = GetActorTransform();
	Transform.SetLocation(Transform.GetLocation() + FVector(0, 0, 25));
	AActor* GC = GetWorld()->SpawnActor<AActor>(SpikeGC, Transform);
	if (GC)
	{
		GC->SetReplicates(true);
		GC->SetLifeSpan(5);
	}
}

void ASpikeTrap::OnAttackedByBoss()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	if (HasAuthority())
	{
		ATrapperPlayer* TrapperPlayer = Cast<ATrapperPlayer>(Owner);
		if (TrapperPlayer)
		{
			TrapperPlayer->ClientRPCUpdateTrapHudNum(ETrapType::Spike, false);
		}

		if (SpikeGC)
		{
			SetActorEnableCollision(false);
			SetActorHiddenInGame(true);
			SetLifeSpan(3);

			MulticastRPCSpawnGC();
		}
	}
}

void ASpikeTrap::OnOverlapBeginCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (OtherActor && OtherActor->ActorHasTag("Monster"))
	{
		OverlappingMonsters.Add(OtherActor);
		bOverlappingTrigger = true;

		if (bCanAttack)
		{
			//UE_LOG(LogTrap, Warning, TEXT("OnOverlapBeginCharacter"));
			Attack();
		}
	}
}

void ASpikeTrap::OnOverlapEndCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	// 오버랩중인 몬스터가 벗어난 경우 컨테이너에서 삭제
	if (OtherActor && OverlappingMonsters.Contains(OtherActor))
	{
		OverlappingMonsters.Remove(OtherActor);

		// 더 이상 오버랩 중인 몬스터가 없는 경우
		if (OverlappingMonsters.Num() == 0)
		{
			bOverlappingTrigger = false;
		}
	}
}

void ASpikeTrap::OnOverlapBeginCharacterHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	/*if (!bOverlappingTrigger)
	{
		UE_LOG(LogTrap, Warning, TEXT("!bOverlappingTrigger"));
	}*/

	if (!HasAuthority() || !Cast<ACharacter>(OtherActor) || !bOverlappingTrigger)
	{
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("OnOverlapBeginCharacterHit"));

	if (HitCharacters.Find(OtherActor))
	{
		//UE_LOG(LogTrap, Warning, TEXT("Hit Character find return"));
		return;
	}

	HitCharacters.Add(OtherActor);

	float FinalDamage = Damage * DamageRate;
	auto DamageTypeClass = UDamageType::StaticClass();
	UGameplayStatics::ApplyDamage(OtherActor, FinalDamage, OwnerController.Get(), this, DamageTypeClass);

	CheckKillCount(OwnerController.Get(), this);
}

void ASpikeTrap::Attack()
{
	if (bCanAttack)
	{
		bCanAttack = false;
		bIsSpikeTimeline = true;
		bIsSpikeUp = true;
		SpikeTimeline.PlayFromStart();

		MulticastRPCSpikeUpSound();
	}
}

void ASpikeTrap::SpikeDownTimerCallback()
{
	bIsSpikeUp = false;
	bIsSpikeTimeline = true;
	SpikeTimeline.PlayFromStart();

	MulticastRPCSpikeDownSound();
}

void ASpikeTrap::ReAttackTimerCallback()
{
	bCanAttack = true;

	if (bOverlappingTrigger)
	{
		Attack();
	}

	HitCharacters.Reset();
}

void ASpikeTrap::HandleSpikeProgress(float Value)
{
	//UE_LOG(LogTrap, Warning, TEXT("HandleSpikeProgress"));

	FVector NewLocation = InitialSpikeLocation;

	if (bIsSpikeUp)
	{
		//UE_LOG(LogTrap, Warning, TEXT("bIsSpikeUp"));
		NewLocation.Z += FMath::Lerp(0.0f, 80.f, Value);
	}
	else
	{
		//UE_LOG(LogTrap, Warning, TEXT("bIsSpikeDown"));
		NewLocation.Z -= FMath::Lerp(0.0f, 80.f, Value);
	}

	SpikeMesh->SetRelativeLocation(NewLocation);
	//UE_LOG(LogTrap, Warning, TEXT("NewLocation %f"), InitialSpikeLocation.Z);
}

void ASpikeTrap::OnSpikeTimelineFinished()
{
	//UE_LOG(LogTrap, Warning, TEXT("OnSpikeTimelineFinished"));

	bIsSpikeTimeline = false;
	InitialSpikeLocation = SpikeMesh->GetRelativeLocation();

	//UE_LOG(LogTrap, Warning, TEXT("InitialSpikeLocation %f"), InitialSpikeLocation.Z);

	if (bIsSpikeUp)
	{
		GetWorld()->GetTimerManager().SetTimer(SpikeDownTimerHandle, this, &ASpikeTrap::SpikeDownTimerCallback, SpikeUpAndPause, false);
	}
	else
	{
		if (HasAuthority())
		{
			ResetKillCount(this);
		}

		GetWorld()->GetTimerManager().SetTimer(ReAttackTimerHandle, this, &ASpikeTrap::ReAttackTimerCallback, ReAttackTime, false);
	}
}

void ASpikeTrap::MulticastRPCSpikeUpSound_Implementation()
{
	if (SpikeUpSound)
	{
		// 특정 지점에서 사운드 재생
		UGameplayStatics::PlaySoundAtLocation(this, SpikeUpSound, GetActorLocation());
	}
}

void ASpikeTrap::MulticastRPCSpikeDownSound_Implementation()
{
	if (SpikeDownSound)
	{
		// 특정 지점에서 사운드 재생
		UGameplayStatics::PlaySoundAtLocation(this, SpikeDownSound, GetActorLocation());
	}
}

void ASpikeTrap::MulticastRPCSpawnGC_Implementation()
{
	SetActorEnableCollision(false);

	FTransform Transform = GetActorTransform();
	Transform.SetLocation(Transform.GetLocation() + FVector(0, 0, 25));
	AActor* GC = GetWorld()->SpawnActor<AActor>(SpikeGC, Transform);
	if (GC)
	{
		GC->SetLifeSpan(10);
	}
}
