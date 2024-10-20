// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/ClapTrap.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/TrapperPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "TrapperProject.h"
#include "Bow/Arrow.h"
#include "Net/UnrealNetwork.h"
#include "Ping/MarkerComponent/MapMarkerComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/TrapperPlayerMovementComponent.h"
#include "DamageType/DamageTypeKnockBack.h"
#include "Framework/TrapperGameState.h"
#include "Framework/TrapperPlayerController.h"


AClapTrap::AClapTrap()
{
	NiagaraComponentSand = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponentSand"));
	NiagaraComponentSand->SetupAttachment(Mesh);
	NiagaraComponentSand->SetAutoActivate(false);

	NiagaraComponentConnectBlue = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponentConnectBlue"));
	NiagaraComponentConnectBlue->SetupAttachment(Mesh);
	NiagaraComponentConnectBlue->SetAutoActivate(false);

	NiagaraComponentConnectRed = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponentConnectRed"));
	NiagaraComponentConnectRed->SetupAttachment(Mesh);
	NiagaraComponentConnectRed->SetAutoActivate(false);

	MapMarkerwpqkfwpqkf->TrapType = EMapTrapType::ClapAlive;

	PrimaryActorTick.bCanEverTick = true;

	Damage = 150;
	//150
	bIsActiveTrap = false;
	bMagneticTriggerActivated = false;
	bCanAttack = false;
	bIsReInstalled = false;
	bMagneticInteraction = true;
	bIsBump = false;

	Tags.Add(FName("ClapTrap"));
}

void AClapTrap::BeginPlay()
{
	Super::BeginPlay();

	Mesh->OnComponentHit.AddDynamic(this, &AClapTrap::OnHitMagneticArrow);
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AClapTrap::OnOverlapBeginCharacter);
	Mesh->OnComponentHit.AddDynamic(this, &AClapTrap::OnHit);

	OriginalMaterial1 = Mesh->GetMaterial(0);
	OriginalMaterial2 = Mesh->GetMaterial(1);
	OriginalMaterial3 = Mesh->GetMaterial(2);
	OriginalMaterial4 = Mesh->GetMaterial(3);

	WreckageLocation = GetActorLocation();
}

void AClapTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AClapTrap, OtherClap);
}

void AClapTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActiveTrap)
	{
		ActivateTrap();
	}
}

void AClapTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void AClapTrap::ReInstall()
{
	bIsReInstalled = true;
	if (IsValid(OtherClap) && !OtherClap->bIsReInstalled)
	{
		OtherClap->ReInstall();
	}

	bWreckage = false;
	bMagneticTriggerActivated = false;
	ActivatedTriggerToPlayer = nullptr;
	OnHitActors.Reset();
	bMagneticInteraction = true;
	bReActivated = true;

	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block); //Arrow
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Overlap); // TurretProjectile

	ChangeOriginalMaterial();
	ChangeAliveMapMarker();

	FVector2D NiagaraScale = { 70, 30 };
	NiagaraComponentSand->SetVariableVec2(FName("ValueTest"), NiagaraScale);
	OtherClap->NiagaraComponentSand->SetVariableVec2(FName("ValueTest"), NiagaraScale);
}

void AClapTrap::OnAttackedByBoss()
{
	if (bWreckage)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	bIsOnAttackedByBoss = true;
	if (IsValid(OtherClap) && !OtherClap->bIsOnAttackedByBoss)
	{
		OtherClap->OnAttackedByBoss();

		if (HasAuthority())
		{
			if (IsValid(TrapperGameState.Get()))
			{
				TrapperGameState->NextCanPlaceClapTrap(true);
			}

			if (ClapGC)
			{
				SetActorEnableCollision(false);
				SetActorHiddenInGame(true);
				SetLifeSpan(3);

				OtherClap->SetActorEnableCollision(false);
				OtherClap->SetActorHiddenInGame(true);
				OtherClap->SetLifeSpan(3);

				MulticastRPCSpawnGC();
			}
		}
	}
}

void AClapTrap::Demolish()
{
	if (bWreckage)
	{
		return;
	}

	if (IsValid(TrapperGameState.Get()))
	{
		TrapperGameState->NextCanPlaceClapTrap(true);
	}

	AActor* GC = GetWorld()->SpawnActor<AActor>(ClapGC, GetActorTransform());
	if (GC)
	{
		GC->SetReplicates(true);
		GC->SetLifeSpan(5);
	}

	AActor* OtherGC = GetWorld()->SpawnActor<AActor>(ClapGC, OtherClap->GetActorTransform());
	if (OtherGC)
	{
		OtherGC->SetReplicates(true);
		OtherGC->SetLifeSpan(5);
	}
}

void AClapTrap::SetOtherClapInteraction(ATrapBase* OtherTrap)
{
	AClapTrap* Clap = Cast<AClapTrap>(OtherTrap);
	if (IsValid(Clap))
	{
		OtherClap = Clap;
		OtherClap->SetOtherClap(this);

		ConnectionOtherClap(OtherClap->GetActorLocation());
		OtherClap->ConnectionOtherClap(GetActorLocation());

		FVector2D BeamLength = FVector2D(50, abs(FVector::Distance(GetActorLocation(), OtherClap->GetActorLocation())));

		NiagaraComponentConnectBlue->SetVariableVec2(FName("BeamLength"), BeamLength);
		NiagaraComponentConnectRed->SetVariableVec2(FName("BeamLength"), BeamLength);
		OtherClap->NiagaraComponentConnectBlue->SetVariableVec2(FName("BeamLength"), BeamLength);
		OtherClap->NiagaraComponentConnectRed->SetVariableVec2(FName("BeamLength"), BeamLength);
	}
}

void AClapTrap::SetOtherClap(ATrapBase* OtherTrap)
{
	AClapTrap* Clap = Cast<AClapTrap>(OtherTrap);
	if (IsValid(Clap))
	{
		OtherClap = Clap;
	}
}

void AClapTrap::SetWeckageOnTimer()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AClapTrap::SetWeckageOnTimerCallback, 3, false);

}

void AClapTrap::SetMeshBlockOnTimer()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AClapTrap::SetMeshBlockOnTimerCallback, 0.2, false);
}

void AClapTrap::AddOnHitActor(AActor* Actor)
{
	OnHitActors.Add(Actor);
}

void AClapTrap::ConnectionOtherClap(FVector OtherClapLocation)
{
	if (IsValid(OtherClap))
	{
		FVector LocationA = GetActorLocation();
		FVector LocationB = OtherClapLocation;

		FVector Direction = LocationB - LocationA;
		FVector NormalizedDirection = Direction.GetSafeNormal();

		FRotator NewRotation = NormalizedDirection.Rotation();

		NiagaraComponentConnectBlue->SetWorldRotation(NewRotation);
		NiagaraComponentConnectRed->SetWorldRotation(NewRotation);
	}
}

void AClapTrap::MulticastRPCSetTrapEnable_Implementation()
{
	//UE_LOG(LogTrap, Warning, TEXT("MulticastRPCSetTrapEnable_Implementation"));

	OnTrapActivatedBroadcast();

	bIsActiveTrap = true;
	OtherClap->bIsActiveTrap = true;

	bMagneticInteraction = false;
	OtherClap->bMagneticInteraction = false;

	ChangeOriginalMaterial();
	OtherClap->ChangeOriginalMaterial();

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Ignore); // DemolishReInstall
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	OtherClap->Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	OtherClap->Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Ignore); // DemolishReInstall

	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OtherClap->Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	//Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore); //Arrow
	//OtherClap->Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore); //Arrow

	NiagaraComponentConnectBlue->DeactivateImmediate();
	NiagaraComponentConnectRed->DeactivateImmediate();
	OtherClap->NiagaraComponentConnectBlue->DeactivateImmediate();
	OtherClap->NiagaraComponentConnectRed->DeactivateImmediate();

	if (ActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivateSound, GetActorLocation());
		UGameplayStatics::PlaySoundAtLocation(this, ActivateSound, OtherClap->GetActorLocation());
	}
}

void AClapTrap::OnHitMagneticArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("OnOverlapBeginMagneticArrow()"));

	if (OtherActor && OtherActor->ActorHasTag("MagneticArrow"))
	{
		AArrow* Arrow = Cast<AArrow>(OtherActor);
		if (IsValid(Arrow) && Arrow->IsActivated())
		{
			Arrow->DeactivateArrow();
		}

		if (!OtherClap || !bMagneticInteraction)
		{
			return;
		}

		AActor* OwnerActor = OtherActor->GetOwner();
		if (!IsValid(OwnerActor))
		{
			return;
		}

		if (!bReActivated)
		{
			return;
		}

		bool bIsServer = false;
		if (Arrow->IsSpawnedBy1P())
		{
			//UE_LOG(LogTrap, Warning, TEXT("1P NiagaraSystem"));
			bIsServer = true;
		}
		else
		{
			//UE_LOG(LogTrap, Warning, TEXT("2P NiagaraSystem"));
			bIsServer = false;
		}

		MulticastRPCSpawnNiagaraSystem(bIsServer, Hit.ImpactPoint);
		MulticastRPCChangeMaterial(bIsServer);

		// 나를 발동시킨 플레이어와 현재 발동시키려는 플레이어가 동일하다면 리턴
		if (IsValid(ActivatedTriggerToPlayer) && ActivatedTriggerToPlayer == OwnerActor)
		{
			ClearTriggerTimer();
			return;
		}

		// 다른 박수를 발동시킨 플레이어가 현재 자신을 발동시킨 플레이어와 동일하다면 리턴
		AActor* OtherClapActivatedToPlayer = OtherClap->GetActivatedTriggerToPlayer();
		if (IsValid(OtherClapActivatedToPlayer))
		{
			if (OtherClapActivatedToPlayer == OwnerActor)
			{
				OtherClap->MulticastRPCChangeOriginalMaterial();
				OtherClap->ActivatedTriggerToPlayer = nullptr;
				OtherClap->bMagneticTriggerActivated = false;
				OtherClap->ClearTimer();
			}
		}

		// 에임 피드백
		AimFeedback(OtherActor);

		// 트리거 활성화
		bMagneticTriggerActivated = true;
		ActivatedTriggerToPlayer = OwnerActor;

		// 두 트리거 모두 활성화되면 함정을 발동
		if (OtherClap->IsMagneticTriggerActivated())
		{
			//UE_LOG(LogTrap, Warning, TEXT("OtherClap->IsMagneticTriggerActivated() = true"));

			ATrapperPlayer* Player = Cast<ATrapperPlayer>(OtherActor->GetOwner());
			if (Player)
			{
				TriggeredByController = Player->GetController();
				OtherClap->TriggeredByController = TriggeredByController;
			}

			TrapperGameState->AddScore(EScoreType::TrapScoreAll, 100);

			//TrapperGameState->NextCanPlcaeClapTrap(true);

			bCanAttack = true;
			MulticastRPCSetTrapEnable();

			return;
		}

		ClearTriggerTimer();

	}
}

void AClapTrap::OnOverlapBeginCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bIsActiveTrap)
	{
		//UE_LOG(LogTrap, Warning, TEXT("bIsActiveTrap false"));
		return;
	}

	// 몬스터와 플레이어는 데미지를 받는다.
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		// 이미 데미지를 받은 액터는 스킵
		if (OtherActor && OnHitActors.Contains(OtherActor))
		{
			return;
		}

		//UE_LOG(LogTrap, Warning, TEXT("OtherActor Damage"));

		//if (OtherActor->ActorHasTag("Player"))
		//{
		//	//UE_LOG(LogTrap, Warning, TEXT("Player"));
		//	ACharacter* Character = Cast<ACharacter>(OtherActor);
		//	if (Character)
		//	{
		//		//UE_LOG(LogTrap, Warning, TEXT("Character"));
		//		UTrapperPlayerMovementComponent* Movement = Cast<UTrapperPlayerMovementComponent>(Character->GetMovementComponent());
		//		if (Movement)
		//		{
		//			//UE_LOG(LogTrap, Warning, TEXT("Movement"));
		//			if (Movement->GetMagneticMovingState())
		//			{
		//				//UE_LOG(LogTrap, Warning, TEXT("Movement->bIsMagneticMovingCast"));
		//				Movement->StopMagneticMove();
		//			}
		//		}
		//	}
		//}

		float FinalDamage = Damage * DamageRate;

		UClass* DamageTypeClass = nullptr;

		if (OtherActor->ActorHasTag("Boss"))
		{
			DamageTypeClass = UDamageType::StaticClass();
		}
		else
		{
			DamageTypeClass = UDamageTypeKnockBack::StaticClass();
		}

		UGameplayStatics::ApplyDamage(OtherActor, FinalDamage, TriggeredByController.Get(), this, DamageTypeClass);

		// 타격을 입은 액터는 컨테이너에 추가
		OnHitActors.Add(OtherActor);
		OtherClap->AddOnHitActor(OtherActor);

		if (OtherActor->ActorHasTag("Boss"))
		{
			return;
		}

		FVector LaunchDirection = (OtherActor->GetActorUpVector() * 600) + ((OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal() * 1500);
		LaunchDirection = LaunchDirection.GetSafeNormal();

		//FVector LaunchDirection = (OtherActor->GetActorUpVector() * 300) + ((OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal() * 1500);


		ACharacter* Character = Cast<ACharacter>(OtherActor);
		bool bCanLaunch = true;
		if (OtherActor->ActorHasTag("Player"))
		{
			if (Character)
			{
				UTrapperPlayerMovementComponent* Movement = Cast<UTrapperPlayerMovementComponent>(Character->GetMovementComponent());
				if (Movement)
				{
					if (Movement->GetMagneticMovingState())
					{
						//Movement->StopMagneticMove();

						bCanLaunch = false;
					}
				}
			}
		}

		if (bCanLaunch)
		{
			if (Character)
			{
				Character->LaunchCharacter(LaunchDirection * Strength, true, true);
			}
		}
	}
}

void AClapTrap::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//if (!HasAuthority())
	//{
	//	return;
	//}

	//UE_LOG(LogTrap, Warning, TEXT("OtherActor Name %s"), *OtherActor->GetName());

	//if (!bCanAttack)
	//{
	//	return;
	//}

	//// 몬스터와 플레이어는 데미지를 받는다.
	//if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()) && bIsActiveTrap)
	//{
	//	// 이미 데미지를 받은 액터는 스킵
	//	if (OtherActor && OnHitActors.Contains(OtherActor))
	//	{
	//		return;
	//	}

	//	// 넉백 처리
	//	/*FVector LaunchDirection = (OtherActor->GetActorUpVector() * 800) + ((OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal() * -1500);
	//	Cast<ACharacter>(OtherActor)->LaunchCharacter(LaunchDirection, false, false);*/

	//	// 액터에 데미지 전달
	//	auto DamageTypeClass = UDamageType::StaticClass();
	//	UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, this, DamageTypeClass);

	//	// 타격을 입은 액터는 컨테이너에 추가
	//	OnHitActors.Add(OtherActor);
	//	OtherClap->AddOnHitActor(OtherActor);
	//}
}

void AClapTrap::ActivateTrap()
{
	NiagaraComponentSand->Activate(true);
	OtherClap->NiagaraComponentSand->Activate(true);

	//FVector MyLocation = GetActorLocation();
	//FVector OtherLocation = OtherClap->GetActorLocation();

	FVector MyLocation = Mesh->GetComponentLocation();
	FVector OtherLocation = OtherClap->Mesh->GetComponentLocation();

	// Lerp를 사용하여 함정을 서로의 위치로 점진적으로 이동
	FVector NewLocation = FMath::Lerp(MyLocation, OtherLocation, GetWorld()->GetDeltaSeconds() * Speed / FVector::Distance(MyLocation, OtherLocation));
	//FVector NewLocation = FMath::Lerp(MyLocation, OtherLocation,  GetWorld()->GetDeltaSeconds() * 1000 / FMath::Pow(FVector::Distance(MyLocation, OtherLocation) / 10, 1.5));

	//SetActorLocation(NewLocation);
	Mesh->SetWorldLocation(NewLocation);

	// 함정이 서로의 위치에 도달하면 중지
	if (FVector::Distance(NewLocation, OtherLocation) < 180.f) // 특정 임계값 이하로 도달하면
	{
		bIsActiveTrap = false;
		OtherClap->SetActiveTrap(false);

		/*EffectSystem->SetWorldScale3D(FVector(100.f, 100.f, 100.f));
		OtherClap->EffectSystem->SetWorldScale3D(FVector(100.f, 100.f, 100.f));*/

		if (BumpSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, BumpSound, GetActorLocation());

			FVector2D NiagaraScale = { 150,70 };
			NiagaraComponentSand->SetVariableVec2(FName("ValueTest"), NiagaraScale);
			OtherClap->NiagaraComponentSand->SetVariableVec2(FName("ValueTest"), NiagaraScale);

			NiagaraComponentSand->Activate(true);
			OtherClap->NiagaraComponentSand->Activate(true);
		}

		ClearTimer();
		OtherClap->ClearTimer();

		//UE_LOG(LogTrap, Warning, TEXT("Distance"));

		bCanAttack = false;
		OtherClap->SetWeckageOnTimer();
		SetWeckageOnTimer();

		SetMeshBlockOnTimer();
		OtherClap->SetMeshBlockOnTimer();

		CheckAndResetKillCountCollaborationForClapTrap();

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ActivateNiagara,
			(Mesh->GetComponentLocation() + OtherClap->Mesh->GetComponentLocation()) / 2.0f,
			FRotator::ZeroRotator,
			FVector(1.0f),
			true,
			true,
			ENCPoolMethod::None,
			true
		);
	}

	/*EffectSystem->Activate(true);
	OtherClap->EffectSystem->Activate(true);*/

	/*if (HasAuthority())
	{
		UE_LOG(LogTrap, Warning, TEXT("Actor Name : %s"), *GetName());
		UE_LOG(LogTrap, Warning, TEXT("Location : %s"), *MyLocation.ToString());
		UE_LOG(LogTrap, Warning, TEXT("OtherLocation : %s"), *OtherLocation.ToString());
		UE_LOG(LogTrap, Warning, TEXT("NewLocation : %s"), *NewLocation.ToString());

		float Distance = FVector::Distance(NewLocation, OtherLocation);
		UE_LOG(LogTrap, Warning, TEXT("FVector::Distance(NewLocation, OtherLocation) : %f"), Distance);

		UE_LOG(LogTrap, Warning, TEXT("--------------------------------"));
	}*/
}

void AClapTrap::PlayMagneticSound()
{
	// 사운드
	if (MagneticSound)
	{
		// 특정 지점에서 사운드 재생
		UGameplayStatics::PlaySoundAtLocation(this, MagneticSound, GetActorLocation());
	}
}

void AClapTrap::ClearTriggerTimer()
{
	ClearTimer();

	GetWorld()->GetTimerManager().SetTimer(ClearTriggerTimerHandle, this, &AClapTrap::ClearTriggerTimerCallback, 5.f, false);
}

void AClapTrap::ChangeOriginalMaterial()
{
	//UE_LOG(LogTrap, Warning, TEXT("ChangeOriginalMaterial"));
	Mesh->SetMaterial(0, OriginalMaterial1);
	Mesh->SetMaterial(1, OriginalMaterial2);
	Mesh->SetMaterial(2, OriginalMaterial3);
	Mesh->SetMaterial(3, OriginalMaterial4);
}

void AClapTrap::WreckageActivation()
{
	//SetActorLocation(WreckageLocation);
	Mesh->SetWorldLocation(WreckageLocation);

	bWreckage = true;
	bIsReInstalled = false;

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Block); // DemolishReInstall

	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore); //Arrow

	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Ignore); // TurretProjectile

	ChangeWreckageMaterial(Mesh);
	ChangeWreckageMapMarker();
}

void AClapTrap::ClearTimer()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ClearTriggerTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearTriggerTimerHandle);
	}
}

void AClapTrap::CheckAndResetKillCountCollaborationForClapTrap()
{
	if (!OtherClap)
	{
		return;
	}

	if (IsValid(TrapperGameState.Get()))
	{
		TrapperGameState->CheckAndResetKillCountCollaborationForClapTrap(this, OtherClap);
	}
}

void AClapTrap::ClearTriggerTimerCallback()
{
	//UE_LOG(LogTrap, Warning, TEXT("ClearTriggerTimer"));
			//UE_LOG(LogTrap, Warning, TEXT("ClearTriggerTimer"));
	ActivatedTriggerToPlayer = nullptr;
	bMagneticTriggerActivated = false;
	if (OtherClap)
	{
		OtherClap->ActivatedTriggerToPlayer = nullptr;
		OtherClap->bMagneticTriggerActivated = false;
		OtherClap->bReActivated = false;
	}

	bReActivated = false;
	GetWorld()->GetTimerManager().SetTimer(ReActivatedTimerHandle, this, &AClapTrap::ReActivatedCallback, 3, false);

	MulticastRPCChangeOriginalMaterial();
}

void AClapTrap::ReActivatedCallback()
{
	bReActivated = true;
	if (OtherClap)
	{
		OtherClap->bReActivated = true;
	}
}

void AClapTrap::SetWeckageOnTimerCallback()
{
	WreckageActivation();
}

void AClapTrap::SetMeshBlockOnTimerCallback()
{
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void AClapTrap::OnRep_OtherClap()
{
	ConnectionOtherClap(OtherClap->GetActorLocation());
	OtherClap->ConnectionOtherClap(GetActorLocation());

	FVector2D BeamLength = FVector2D(50, abs(FVector::Distance(GetActorLocation(), OtherClap->GetActorLocation())));
	NiagaraComponentConnectBlue->SetVariableVec2(FName("BeamLength"), BeamLength);
	NiagaraComponentConnectRed->SetVariableVec2(FName("BeamLength"), BeamLength);
	OtherClap->NiagaraComponentConnectBlue->SetVariableVec2(FName("BeamLength"), BeamLength);
	OtherClap->NiagaraComponentConnectRed->SetVariableVec2(FName("BeamLength"), BeamLength);
}

void AClapTrap::MulticastRPCSpawnGC_Implementation()
{
	SetActorEnableCollision(false);
	OtherClap->SetActorEnableCollision(false);

	AActor* GC = GetWorld()->SpawnActor<AActor>(ClapGC, Mesh->GetComponentTransform());
	if (GC)
	{
		GC->SetLifeSpan(10);
	}

	AActor* OtherGC = GetWorld()->SpawnActor<AActor>(ClapGC, OtherClap->Mesh->GetComponentTransform());
	if (OtherGC)
	{
		OtherGC->SetLifeSpan(10);
	}
}

void AClapTrap::MulticastRPCSpawnNiagaraSystem_Implementation(bool IsServer, const FVector& ImpactPoint)
{
	UNiagaraSystem* NiagaraSystem;
	if (IsServer)
	{
		NiagaraSystem = RedNiagara;

		if (NiagaraComponentConnectRed)
		{
			NiagaraComponentConnectRed->Activate(true);
		}

		if (NiagaraComponentConnectBlue)
		{
			if (NiagaraComponentConnectBlue->IsActive())
			{
				NiagaraComponentConnectBlue->DeactivateImmediate();
			}
		}
	}
	else
	{
		NiagaraSystem = BlueNiagara;

		if (NiagaraComponentConnectBlue)
		{
			NiagaraComponentConnectBlue->Activate(true);
		}

		if (NiagaraComponentConnectRed)
		{
			if (NiagaraComponentConnectRed->IsActive())
			{
				NiagaraComponentConnectRed->DeactivateImmediate();
			}
		}
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		NiagaraSystem,
		ImpactPoint,
		FRotator::ZeroRotator,
		FVector(1.0f),
		true,
		true,
		ENCPoolMethod::None,
		true
	);

	PlayMagneticSound();

}

void AClapTrap::MulticastRPCChangeMaterial_Implementation(bool IsServer)
{
	//UE_LOG(LogTrap, Warning, TEXT("MulticastRPCChangeMaterial_Implementation"));
	if (IsServer)
	{
		Mesh->SetMaterial(0, MaterialRed);
		Mesh->SetMaterial(1, MaterialRed);
		Mesh->SetMaterial(2, MaterialRed);
		Mesh->SetMaterial(3, MaterialRed);
	}
	else
	{
		Mesh->SetMaterial(0, MaterialBlue);
		Mesh->SetMaterial(1, MaterialBlue);
		Mesh->SetMaterial(2, MaterialBlue);
		Mesh->SetMaterial(3, MaterialBlue);
	}
}

void AClapTrap::MulticastRPCChangeOriginalMaterial_Implementation()
{
	ChangeOriginalMaterial();

	NiagaraComponentConnectBlue->DeactivateImmediate();
	NiagaraComponentConnectRed->DeactivateImmediate();
}
