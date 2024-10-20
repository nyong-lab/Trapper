// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/GunpowderBarrelTrap.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "TrapperProject.h"
#include "Bow/Arrow.h"
#include "Character/TrapperPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Ping/MarkerComponent/MapMarkerComponent.h"
#include "Trap/Component/TrapMechanics.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Character/TrapperPlayerMovementComponent.h"
#include "Widget/MultikillHUD.h"
#include "Framework/TrapperGameState.h"
#include "DamageType/DamageTypeDown.h"


AGunpowderBarrelTrap::AGunpowderBarrelTrap()
{
	MapMarkerwpqkfwpqkf->TrapType = EMapTrapType::BarrelAlive;

	//UE_LOG(LogTrap, Warning, TEXT("AGunpowderBarrelTrap()"));
	//if (IsValid(MapMarker))
	//{
	//	UE_LOG(LogTrap, Warning, TEXT("MapMarker true"));
	//}
	//else
	//{
	//	UE_LOG(LogTrap, Warning, TEXT("MapMarker false"));
	//}

	ExplosionTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionTrigger"));
	ExplosionTrigger->SetupAttachment(Mesh);

	NiagaraComponentExplosion = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponentExplosion"));
	NiagaraComponentExplosion->SetupAttachment(DummyRoot);
	NiagaraComponentExplosion->SetAutoActivate(false);

	Damage = 75;

	bExplodedBarrel = false;

	Tags.Add(FName("GunpowderBarrelTrap"));
}

void AGunpowderBarrelTrap::BeginPlay()
{
	Super::BeginPlay();

	ExplosionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AGunpowderBarrelTrap::OnOverlapBeginExplosionTrigger);
	ExplosionTrigger->OnComponentEndOverlap.AddDynamic(this, &AGunpowderBarrelTrap::OnOverlapEndExplosionTrigger);

	Mesh->OnComponentHit.AddDynamic(this, &AGunpowderBarrelTrap::OnHit);
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AGunpowderBarrelTrap::OnOverlapBeginTrap);
	//Trigger->OnComponentBeginOverlap.AddDynamic(this, &AGunpowderBarrelTrap::OnOverlapBeginArrow);
	//EffectSystem->OnSystemFinished.AddDynamic(this, &AGunpowderBarrelTrap::OnEffectFinished);

	NiagaraComponentExplosion->OnSystemFinished.AddDynamic(this, &AGunpowderBarrelTrap::OnNiagaraSystemFinished);
}

void AGunpowderBarrelTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void AGunpowderBarrelTrap::Demolish()
{
	if (bWreckage)
	{
		return;
	}

	AActor* GC = GetWorld()->SpawnActor<AActor>(GunpowderBarrelGC, GetActorTransform());
	if (GC)
	{
		GC->SetReplicates(true);
		GC->SetLifeSpan(5);
	}
}

void AGunpowderBarrelTrap::ReInstall()
{
	Mesh->SetVisibility(true);
	ECollisionResponse BlockResponse = ECollisionResponse::ECR_Block;
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, BlockResponse);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, BlockResponse);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Arrow
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, BlockResponse); // DemolishReInstall
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Overlap); // TurretProjectile
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ExplosionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	Mesh->SetCollisionResponseToChannel(ECC_Camera, BlockResponse);

	bWreckage = false;
	bExplodedBarrel = false;
	bTriggeredExternal = false;
	bSafeExplosionBarrelsCallback = false;
	bSafeExplosionBarrelsCallbackReal = false;

	ChangeOriginalMaterial();
	ChangeAliveMapMarker();
}

void AGunpowderBarrelTrap::OnAttackedByBoss()
{
	if (bWreckage)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	if (HasAuthority())
	{
		ATrapperPlayer* TrapperPlayer = Cast<ATrapperPlayer>(Owner);
		if (TrapperPlayer)
		{
			TrapperPlayer->ClientRPCUpdateTrapHudNum(ETrapType::GunpowderBarrel, false);
		}

		if (GunpowderBarrelGC)
		{
			SetActorEnableCollision(false);
			SetActorHiddenInGame(true);
			SetLifeSpan(3);

			MulticastRPCSpawnGC();
		}
	}
}

void AGunpowderBarrelTrap::OnOverlapBeginExplosionTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(OtherActor))
	{
		return;
	}

	// 몬스터와 플레이어는 일단 TSet에 모아둠
	if (OtherActor->ActorHasTag("Monster") || OtherActor->ActorHasTag("Player"))
	{
		if (!OverlappingActors.Contains(OtherActor))
		{
			if (bSafeRangedFor)
			{
				//UE_LOG(LogTrap, Error, TEXT("GunpowderBarrelTrap : bSafeRangedFor true -> Add Actor"));
				SafeAddActorArray.Add(OtherActor);
			}
			else
			{
				OverlappingActors.Add(OtherActor);
			}
		}
	}
	else if (OtherActor->ActorHasTag("GunpowderBarrelTrap"))
	{
		if (OtherActor == this)
		{
			return;
		}

		/*if (OtherComp->ComponentHasTag("BarrelMesh"))
		{
			UE_LOG(LogTrap, Warning, TEXT("Test Begin BarrelMesh"));
		}*/

		if (OtherComp->ComponentHasTag("ExplosionTrigger") ||
			OtherComp->ComponentHasTag("BarrelMesh"))
		{
			return;
		}

		if (OtherComp->ComponentHasTag("Please"))
		{
			if (OtherComp->GetOwner() == this)
			{
				return;
			}

			if (!OverlappingBarrels.Contains(OtherActor))
			{
				UE_LOG(LogTrap, Warning, TEXT("------------------------------------ Begin ------------------------------------"));
				UE_LOG(LogTrap, Warning, TEXT("MyName : %s"), *GetName());
				UE_LOG(LogTrap, Warning, TEXT("Begin -> OtherActor Name : %s"), *OtherActor->GetName());
				if (bSafeRangedFor)
				{
					//UE_LOG(LogTrap, Error, TEXT("GunpowderBarrelTrap : bSafeRangedFor true -> Add Barrel"));
					SafeAddBarrelArray.Add(OtherActor);
				}
				else
				{
					OverlappingBarrels.Add(OtherActor);
				}
			}
		}
	}
}

void AGunpowderBarrelTrap::OnOverlapEndExplosionTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(OtherActor))
	{
		return;
	}

	if (OverlappingActors.Contains(OtherActor))
	{
		if (bSafeRangedFor)
		{
			//UE_LOG(LogTrap, Error, TEXT("GunpowderBarrelTrap : bSafeRangedFor true -> Remove Actor"));
			SafeRemoveActorArray.Add(OtherActor);
		}
		else
		{
			OverlappingActors.Remove(OtherActor);
		}
	}
	else if (OverlappingBarrels.Contains(OtherActor))
	{
		if (OtherActor == this)
		{
			return;
		}

		if (OtherComp->ComponentHasTag("ExplosionTrigger") ||
			OtherComp->ComponentHasTag("BarrelMesh"))
		{
			return;
		}

		if (OtherComp->ComponentHasTag("Please"))
		{	
			if (OtherComp->GetOwner() == this)
			{
				return;
			}

			if (OverlappingBarrels.Contains(OtherActor))
			{
				UE_LOG(LogTrap, Warning, TEXT("------------------------------------ End ------------------------------------"));
				UE_LOG(LogTrap, Warning, TEXT("MyName : %s"), *GetName());
				UE_LOG(LogTrap, Warning, TEXT("End -> OtherActor Name : %s"), *OtherActor->GetName());

				if (bSafeRangedFor)
				{
					//UE_LOG(LogTrap, Error, TEXT("GunpowderBarrelTrap : bSafeRangedFor true -> Remove Barrel"));
					SafeRemoveBarrelArray.Add(OtherActor);
				}
				else
				{
					OverlappingBarrels.Remove(OtherActor);
				}
			}
		}
	}
}

void AGunpowderBarrelTrap::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || bWreckage)
	{
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("AGunpowderBarrelTrap::OnHit"));

	if (!IsValid(OtherActor))
	{
		return;
	}

	if (OtherActor->ActorHasTag("DefaultArrow") || OtherActor->ActorHasTag("MagneticArrow"))
	{
		AArrow* Arrow = Cast<AArrow>(OtherActor);
		if (!IsValid(Arrow))
		{
			return;
		}
		if (Arrow->IsActivated())
		{
			Arrow->DeactivateArrow();
		}

		AActor* OwnerActor = OtherActor->GetOwner();
		if (!OwnerActor)
		{
			return;
		}

		ATrapperPlayer* Player = Cast<ATrapperPlayer>(OwnerActor);
		if (Player)
		{
			TriggeredByController = Player->GetController();
		}

		if (Arrow->IsSpawnedBy1P())
		{
			bTriggered1P = true;
			TrapperGameState->AddScore(EScoreType::TrapScore1P, 50);
		}
		else
		{
			bTriggered1P = false;
			TrapperGameState->AddScore(EScoreType::TrapScore2P, 50);
		}

		// 에임 피드백
		AimFeedback(OtherActor);
		ActivateTrap();
	}
	else if (OtherActor->ActorHasTag("PlankTrap"))
	{
		bTriggeredExternal = true;
		ActivateTrap();
	}
}

void AGunpowderBarrelTrap::OnOverlapBeginTrap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	if (!HasAuthority() || bWreckage)
	{
		return;
	}

	if (OtherActor->ActorHasTag("GuillotinePendulumTrap") ||
		OtherActor->ActorHasTag("ClapTrap") ||
		OtherActor->ActorHasTag("SpearTrap"))
	{
		bTriggeredExternal = true;
		ActivateTrap();
	}
}

void AGunpowderBarrelTrap::OnNiagaraSystemFinished(UNiagaraComponent* FinishedComponent)
{
	//UE_LOG(LogTemp, Warning, TEXT("OnNiagaraSystemFinished"));

	ECollisionResponse NewResponse = ECollisionResponse::ECR_Block;
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, NewResponse);

	WreckageActivation();
}

void AGunpowderBarrelTrap::ActivateTrap()
{
	if (bExplodedBarrel)
	{
		return;
	}

	/*ATrapperPlayer* TrapOwnerPlayer = Cast<ATrapperPlayer>(GetOwner());
	if (TrapOwnerPlayer)
	{
		TrapOwnerPlayer->ClientRPCUpdateTrapHudNum(ETrapType::GunpowderBarrel, false);
	}*/

	bExplodedBarrel = true;

	FVector ExplosionOrigin = GetActorLocation();
	float ExplosionRadius = ExplosionTrigger->GetScaledSphereRadius();
	//float MaxImpulseStrength = RadialForceComponent->ImpulseStrength;
	float MinImpulseStrength = 300.f;

	bSafeRangedFor = true;
	for (AActor* Actor : OverlappingActors)
	{
		if (IsValid(Actor))
		{
			ACharacter* Character = Cast<ACharacter>(Actor);
			if (Character)
			{
				//UE_LOG(LogTrap, Warning, TEXT("Character Name : %s"), *Character->GetName());

				//if (Actor->ActorHasTag("Player"))
				//{
				//	UTrapperPlayerMovementComponent* Movement = Cast<UTrapperPlayerMovementComponent>(Character->GetMovementComponent());
				//	if (Movement)
				//	{
				//		if (Movement->GetMagneticMovingState())
				//		{
				//			//UE_LOG(LogTrap, Warning, TEXT("MulticastRPCStopMagneticMove"));
				//			Movement->StopMagneticMove();
				//		}
				//	}
				//}

				float FinalDamage = Damage * DamageRate;

				UClass* DamageTypeClass = nullptr;

				if (Actor->ActorHasTag("Boss"))
				{
					DamageTypeClass = UDamageType::StaticClass();
				}
				else
				{
					DamageTypeClass = UDamageTypeDown::StaticClass();
				}

				UGameplayStatics::ApplyDamage(Character, FinalDamage, TriggeredByController.Get(), this, DamageTypeClass);

				if (Actor->ActorHasTag("Boss"))
				{
					continue;
				}

				// 화약통과의 거리 및 방향 계산
				FVector LaunchDirection = Character->GetActorLocation() - ExplosionOrigin;
				float Distance = LaunchDirection.Size();
				LaunchDirection += Character->GetActorUpVector() * 100;
				LaunchDirection.Normalize();

				// 거리가 폭발 반경을 초과하는 경우 처리하지 않음
				if (Distance > ExplosionRadius)
				{
					continue;
				}

				// s거리에 따라 힘의 감소를 적용하여 Strength 계산
				//float Strength = MaxImpulseStrength * (1.0f - Distance / ExplosionRadius); // 거리 비례 힘 감소
				float FinalStrength = FMath::Max(Strength * FMath::Sqrt(1.0f - Distance / ExplosionRadius), MinImpulseStrength);

				//UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());
				//UE_LOG(LogTrap, Warning, TEXT("FinalStrength : %f"), FinalStrength);

				// AirControl
				// Mass
				// Gravity Scale

				Character->GetCharacterMovement()->BrakingDecelerationFalling = 0;

				Character->LaunchCharacter(LaunchDirection * FinalStrength, true, true);


				//UE_LOG(LogTrap, Warning, TEXT("LaunchDirection %s"), *LaunchDirection.ToString());
				//UE_LOG(LogTrap, Warning, TEXT("LaunchDirection * Strength%s"), *((LaunchDirection * Strength).ToString()));

				//UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());
				//UE_LOG(LogTrap, Warning, TEXT("Distance : %f"), Distance);
				//UE_LOG(LogTrap, Warning, TEXT("Strength : %f"), Strength);
				//UE_LOG(LogTrap, Warning, TEXT("Actor Name : %s"), *Actor->GetName());
				//UE_LOG(LogTrap, Warning, TEXT("LaunchDirection * Strength : %s"), *(LaunchDirection * Strength).ToString());
			}
		}
	}
	bSafeRangedFor = false;

	for (const auto& SafeAddActor : SafeAddActorArray)
	{
		//UE_LOG(LogTrap, Error, TEXT("GunpowderBarrelTrap : SafeAddActorArray"));

		if (!OverlappingActors.Contains(SafeAddActor))
		{
			OverlappingActors.Add(SafeAddActor);
		}
	}
	SafeAddActorArray.Empty();

	for (const auto& SafeRemoveActor : SafeRemoveActorArray)
	{
		//UE_LOG(LogTrap, Error, TEXT("GunpowderBarrelTrap : SafeRemoveActorArray"));

		if (OverlappingActors.Contains(SafeRemoveActor))
		{
			OverlappingActors.Remove(SafeRemoveActor);
		}
	}
	SafeRemoveActorArray.Empty();

	for (const auto& SafeAddBarrel : SafeAddBarrelArray)
	{
		//UE_LOG(LogTrap, Error, TEXT("GunpowderBarrelTrap : SafeAddBarrelArray"));

		if (!OverlappingBarrels.Contains(SafeAddBarrel))
		{
			OverlappingBarrels.Add(SafeAddBarrel);
		}
	}
	SafeAddBarrelArray.Empty();

	for (const auto& SafeRemoveBarrel : SafeRemoveBarrelArray)
	{
		//UE_LOG(LogTrap, Error, TEXT("GunpowderBarrelTrap : SafeRemoveBarrelArray"));

		if (OverlappingBarrels.Contains(SafeRemoveBarrel))
		{
			OverlappingBarrels.Remove(SafeRemoveBarrel);
		}
	}
	SafeRemoveBarrelArray.Empty();

	CheckAndResetKillCount(TriggeredByController.Get(), this);

	if (!bSafeExplosionBarrelsCallback)
	{
		bSafeExplosionBarrelsCallback = true;
		ExplosionBarrels = OverlappingBarrels;

		/*bSafeRangedFor = true;
		for (const auto& Barrel : OverlappingBarrels)
		{
			if (ExplosionBarrels.Contains(Barrel))
			{
				ExplosionBarrels.Add(Barrel);
			}
		}
		bSafeRangedFor = false;

		for (const auto& SafeAddBarrel : SafeAddBarrelArray)
		{
			UE_LOG(LogTrap, Error, TEXT("GunpowderBarrelTrap : SafeAddBarrelArray"));

			if (!OverlappingBarrels.Contains(SafeAddBarrel))
			{
				OverlappingBarrels.Add(SafeAddBarrel);
			}
		}
		SafeAddBarrelArray.Empty();

		for (const auto& SafeRemoveBarrel : SafeRemoveBarrelArray)
		{
			UE_LOG(LogTrap, Error, TEXT("GunpowderBarrelTrap : SafeRemoveBarrelArray"));

			if (OverlappingBarrels.Contains(SafeRemoveBarrel))
			{
				OverlappingBarrels.Remove(SafeRemoveBarrel);
			}
		}
		SafeRemoveBarrelArray.Empty();*/

		GetWorld()->GetTimerManager().SetTimer(ExplosionTimerHandle, this, &AGunpowderBarrelTrap::OverlappingBarrelsExplosionCallback, 0.25, false);
	}
	else
	{
		UE_LOG(LogTrap, Warning, TEXT("bSafeExplosionBarrelsCallback true !!!!!!!!!!!!!!"));
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, FString::Printf(TEXT("bSafeExplosionBarrelsCallback true !!!!!!!!!!!!!!")));
		return;
	}

	MulticastRPCActivateEffect();
}

void AGunpowderBarrelTrap::WreckageActivation()
{
	bWreckage = true;

	ChangeWreckageMaterial(Mesh);
	ChangeWreckageMapMarker();

	Mesh->SetVisibility(true);
}

void AGunpowderBarrelTrap::ChangeOriginalMaterial()
{
	Mesh->SetMaterial(0, OriginalMaterial);
}

void AGunpowderBarrelTrap::NiagaraDeactivateCallback()
{
	if (NiagaraComponentExplosion)
	{
		NiagaraComponentExplosion->DeactivateImmediate();
	}
}

void AGunpowderBarrelTrap::OverlappingBarrelsExplosionCallback()
{
	if (!bSafeExplosionBarrelsCallbackReal)
	{
		bSafeExplosionBarrelsCallbackReal = true;
		//TSet<TObjectPtr<AActor>> ExplosionBarrelsCopy = ExplosionBarrels;
		TSet<TObjectPtr<AActor>> ExplosionBarrelsCopy(ExplosionBarrels);
		for (AActor* Actor : ExplosionBarrelsCopy)
		{
			if (IsValid(Actor) && !Actor->IsPendingKill())
			{
				AGunpowderBarrelTrap* Barrel = Cast<AGunpowderBarrelTrap>(Actor);
				if (Barrel)
				{
					Barrel->ActivateTrap();
				}
			}
		}

		ExplosionBarrels.Empty();
	}
	else
	{
		UE_LOG(LogTrap, Warning, TEXT("AGunpowderBarrelTrap::OverlappingBarrelsExplosionCallback() -> bSafeExplosionBarrelsCallbackReal true !!!!!!!!!!!!!!"));
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, FString::Printf(TEXT("AGunpowderBarrelTrap::OverlappingBarrelsExplosionCallback() -> bSafeExplosionBarrelsCallbackReal true !!!!!!!!!!!!!!")));
	}
}

void AGunpowderBarrelTrap::NextTickDestory()
{
	Destroy();
}

void AGunpowderBarrelTrap::MulticastRPCActivateEffect_Implementation()
{
	OnTrapActivatedBroadcast();

	// 디버그 드로우를 제거
	//GetWorld()->GetTimerManager().ClearTimer(DebugDrawTimerHandle);

	Mesh->SetVisibility(false);
	ECollisionResponse IgnoreResponse = ECollisionResponse::ECR_Ignore;
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, IgnoreResponse);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, IgnoreResponse);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, IgnoreResponse); // Arrow
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, IgnoreResponse); // DemolishReInstall
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, IgnoreResponse); // TurretProjectile
	ExplosionTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Mesh->SetCollisionResponseToChannel(ECC_Camera, IgnoreResponse);

	//EffectSystem->Activate(true);
	if (NiagaraComponentExplosion)
	{
		NiagaraComponentExplosion->Activate(true);
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AGunpowderBarrelTrap::NiagaraDeactivateCallback, 2, false);

	if (ActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivateSound, GetActorLocation());
	}
}

void AGunpowderBarrelTrap::MulticastRPCSpawnGC_Implementation()
{
	SetActorEnableCollision(false);

	AActor* GC = GetWorld()->SpawnActor<AActor>(GunpowderBarrelGC, GetActorTransform());
	if (GC)
	{
		GC->SetLifeSpan(10);
	}
}
