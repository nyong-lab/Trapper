// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/OilBagTrap.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "TrapperProject.h"
#include "Bow/Arrow.h"
#include "Character/TrapperPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Trap/TrapEffects/Oil.h"
#include "Components/DecalComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Ping/MarkerComponent/MapMarkerComponent.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Framework/TrapperGameState.h"


AOilBagTrap::AOilBagTrap()
{
	Trigger->SetupAttachment(DummyRoot);

	RopeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RopeMesh"));
	RopeMesh->SetupAttachment(DummyRoot);
	RingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMesh"));
	RingMesh->SetupAttachment(DummyRoot);

	MapMarkerwpqkfwpqkf->TrapType = EMapTrapType::OilBagAlive;

	static ConstructorHelpers::FObjectFinder<UMaterial> OilMaterialFinder(TEXT("/Script/Engine.Material'/Game/Trap/Material/M_PreviewMeshBlue.M_PreviewMeshBlue'"));
	if (OilMaterialFinder.Succeeded())
	{
		OilMaterial = OilMaterialFinder.Object;
	}

	/*NiagaraComponentFallingOil = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponentForFallingOil"));
	NiagaraComponentFallingOil->SetupAttachment(DummyRoot);
	NiagaraComponentFallingOil->SetAutoActivate(false);

	NiagaraComponentSpreadingOil = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponentSpreadingOil"));
	NiagaraComponentSpreadingOil->SetupAttachment(DummyRoot);
	NiagaraComponentSpreadingOil->SetAutoActivate(false);*/

	FallingOilNiagaraAttachPointOffset = CreateDefaultSubobject<USceneComponent>(TEXT("FallingOilNiagaraAttachPointOffset"));
	FallingOilNiagaraAttachPointOffset->SetupAttachment(DummyRoot);

	SpreadingOilNiagaraAttachPointOffset = CreateDefaultSubobject<USceneComponent>(TEXT("SpreadingOilNiagaraAttachPointOffset"));
	SpreadingOilNiagaraAttachPointOffset->SetupAttachment(DummyRoot);

	Damage = 0;
	bCanActivate = true;

	Tags.Add(FName("OilBagTrap"));
}

void AOilBagTrap::BeginPlay()
{
	Super::BeginPlay();

	ReInstallTransform = Mesh->GetComponentTransform();

	Mesh->OnComponentHit.AddDynamic(this, &AOilBagTrap::OnHitMeshToArrow);
	Trigger->OnComponentHit.AddDynamic(this, &AOilBagTrap::OnHitRopeToArrow);

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	AsyncLoadHandle1 = Streamable.RequestAsyncLoad(FallingOilPath, [this]()
		{
			//UE_LOG(LogTrap, Warning, TEXT("RequestAsyncLoad true"));

			// 에셋 로드
			UObject* LoadedObject = FallingOilPath.TryLoad();

			if (LoadedObject)
			{
				// 적절한 타입으로 캐스팅
				NiagaraSystemFallingOil = Cast<UNiagaraSystem>(LoadedObject);

				if (NiagaraSystemFallingOil)
				{
					//UE_LOG(LogTrap, Warning, TEXT("FallingOil asset loaded successfully"));

					//NiagaraComponentFallingOil->SetAsset(NiagaraSystemFallingOil);

					NiagaraComponentFallingOil = UNiagaraFunctionLibrary::SpawnSystemAttached(
						NiagaraSystemFallingOil, FallingOilNiagaraAttachPointOffset,
						NAME_None,
						FVector(0.f), FRotator(0.f),
						EAttachLocation::Type::KeepRelativeOffset, false, false);

					if (NiagaraComponentFallingOil)
					{
						NiagaraComponentFallingOil->OnSystemFinished.AddDynamic(this, &AOilBagTrap::OnNiagaraSystemFinished);
					}
				}
			}
		});

	AsyncLoadHandle2 = Streamable.RequestAsyncLoad(SpreadingOilPath, [this]()
		{
			//UE_LOG(LogTrap, Warning, TEXT("RequestAsyncLoad true"));

			// 에셋 로드
			UObject* LoadedObject = SpreadingOilPath.TryLoad();

			if (LoadedObject)
			{
				// 적절한 타입으로 캐스팅
				NiagaraSystemSpreadingOil = Cast<UNiagaraSystem>(LoadedObject);

				if (NiagaraSystemSpreadingOil)
				{
					//UE_LOG(LogTrap, Warning, TEXT("FallingOil asset loaded successfully"));

					//NiagaraComponentSpreadingOil->SetAsset(NiagaraSystemSpreadingOil);

					NiagaraComponentSpreadingOil = UNiagaraFunctionLibrary::SpawnSystemAttached(
						NiagaraSystemSpreadingOil, SpreadingOilNiagaraAttachPointOffset,
						NAME_None,
						FVector(0.f), FRotator(0.f),
						EAttachLocation::Type::KeepRelativeOffset, false, false);

					if (NiagaraComponentSpreadingOil)
					{
						NiagaraComponentSpreadingOil->OnSystemFinished.AddDynamic(this, &AOilBagTrap::OnNiagaraSystemFinished);
					}
				}
			}
		});



}

void AOilBagTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(CheckOilActorVelocityTimerHandle);

	if (AsyncLoadHandle1.IsValid() && AsyncLoadHandle1->IsActive())
	{
		AsyncLoadHandle1->CancelHandle();
	}

	if (AsyncLoadHandle2.IsValid() && AsyncLoadHandle2->IsActive())
	{
		AsyncLoadHandle2->CancelHandle();
	}
}

void AOilBagTrap::OnHitMeshToArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (OtherActor && (OtherActor->ActorHasTag("DefaultArrow") || OtherActor->ActorHasTag("MagneticArrow")))
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

		if (!bCanActivate)
		{
			return;
		}

		if (!OtherActor->GetOwner())
		{
			return;
		}

		if (Arrow->IsSpawnedBy1P())
		{
			TrapperGameState->AddScore(EScoreType::TrapScore1P, 50);
		}
		else
		{
			TrapperGameState->AddScore(EScoreType::TrapScore2P, 50);
		}

		// 에임 피드백
		AimFeedback(OtherActor);

		bCanActivate = false;

		// 트랩 활성화 (바닥에 오일 생성)
		ActivateTrapExplosion();

		// 이펙트
		MulticastRPCActivateEffect();
	}
}

void AOilBagTrap::OnHitRopeToArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("OnHitRopeToArrow"));

	if (OtherActor && (OtherActor->ActorHasTag("DefaultArrow") || OtherActor->ActorHasTag("MagneticArrow")))
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

		if (!bCanActivate)
		{
			return;
		}

		if (!OtherActor->GetOwner())
		{
			return;
		}

		if (Arrow->IsSpawnedBy1P())
		{
			TrapperGameState->AddScore(EScoreType::TrapScore1P, 50);
		}
		else
		{
			TrapperGameState->AddScore(EScoreType::TrapScore2P, 50);
		}

		// 에임 피드백
		AimFeedback(OtherActor);

		bCanActivate = false;

		if (!Mesh->IsSimulatingPhysics())
		{
			Mesh->SetSimulatePhysics(true);
			Mesh->SetEnableGravity(true);

			MulticastRPCSetCollisionResponseOfSimulating();
		}

		GetWorld()->GetTimerManager().SetTimer(CheckOilActorVelocityTimerHandle, this, &AOilBagTrap::CheckOilVelocity, 0.2f, true);
	}
}

void AOilBagTrap::OnNiagaraSystemFinished(UNiagaraComponent* FinishedComponent)
{
	/*if (HasAuthority())
	{
		UE_LOG(LogTrap, Warning, TEXT("OnNiagaraSystemFinished Name : %s"), *FinishedComponent->GetName());
	}*/

	//Mesh->SetHiddenInGame(true);
	//Mesh->SetVisibility(false);
	//Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AOilBagTrap::ActivateTrapExplosion()
{
	//UE_LOG(LogTrap, Warning, TEXT("AOilBagTrap::ActivateTrap()"));

	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation + FVector(0, 0, -3000);

	//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, true, 1, 0, 1);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	FHitResult HitResult;
	bool bHasHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel8, CollisionParams);

	//UE_LOG(LogTrap, Warning, TEXT("Hit Name : %s"), *HitResult.GetActor()->GetName());

	if (bHasHit)
	{
		// 기름 액터 생성
		//UE_LOG(LogTrap, Warning, TEXT("HasHitName : %s"), *HitResult.GetActor()->GetName());

		TSubclassOf<AOil> OilClass = OilEffect;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		FRotator OilRotator = GetActorRotation();
		OilRotator.Roll = 0;
		OilRotator.Pitch = 0;

		FVector OilLocation = HitResult.Location;
		OilLocation.Z += 1.f;

		FTransform OilTransform;
		OilTransform.SetRotation(OilRotator.Quaternion());
		OilTransform.SetLocation(OilLocation);

		AOil* Oil = GetWorld()->SpawnActorDeferred<AOil>(OilClass, OilTransform, this);
		if (Oil)
		{
			Oil->SetReduceTime(Duration);
			UGameplayStatics::FinishSpawningActor(Oil, OilTransform);
		}

		OilLocation.Z -= 51;
		MulticastRPCSpreadingOil(OilLocation);
	}
}

void AOilBagTrap::ActivateTrapSimulation()
{
	FVector StartLocation = Mesh->GetComponentLocation();
	FVector EndLocation = StartLocation + FVector(0, 0, -200);

	//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, true, 1, 0, 1);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	FHitResult HitResult;
	bool bHasHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel8, CollisionParams);

	if (bHasHit)
	{
		TSubclassOf<AOil> OilClass = OilEffect;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		FTransform OilTransform = Mesh->GetComponentTransform();;

		FRotator OilRotator = OilTransform.GetRotation().Rotator();
		OilRotator.Roll = 0;
		OilRotator.Pitch = 0;
		OilTransform.SetRotation(OilRotator.Quaternion());

		FVector OilLocation = HitResult.Location;
		OilLocation.Z += 1.f;
		OilTransform.SetLocation(OilLocation);

		GetWorld()->SpawnActor<AOil>(OilClass, OilTransform, SpawnParams);

		OilLocation.Z -= 51;
		MulticastRPCSpreadingOil(OilLocation);
	}
}

void AOilBagTrap::CheckOilVelocity()
{
	if (Mesh->GetComponentVelocity().Size() <= 10)
	{
		GetWorld()->GetTimerManager().ClearTimer(CheckOilActorVelocityTimerHandle);
		Mesh->SetSimulatePhysics(false);
		Mesh->SetEnableGravity(false);
		ActivateTrapSimulation();

		/*MulticastRPCSetHiddenMeshOfVelocity();

		FTimerHandle MeshHiddenTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(MeshHiddenTimerHandle, [this]()
			{
			}, 1, false);*/
	}
}

void AOilBagTrap::ChangeOriginalMaterial()
{
	RingMesh->SetMaterial(0, RingOriginalMaterialFirstElement);	
	RingMesh->SetMaterial(1, RingOriginalMaterialSecondElement);
	RopeMesh->SetMaterial(0, RopeAndMeshOriginalMaterialFirstElement);
	Mesh->SetMaterial(0, RopeAndMeshOriginalMaterialFirstElement);
}

void AOilBagTrap::PlaySound()
{
	if (ActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivateSound, GetActorLocation());
	}
}

void AOilBagTrap::HiddenCallback()
{
	Mesh->SetHiddenInGame(true);
}

void AOilBagTrap::FallingOilCallback()
{
	/*if (HasAuthority())
				{
					UE_LOG(LogTrap, Warning, TEXT("NiagaraComponentFallingOil IsActive: %s"), NiagaraComponentFallingOil->IsActive() ? TEXT("True") : TEXT("False"));
					UE_LOG(LogTrap, Warning, TEXT("NiagaraComponentFallingOil Deactivate"));
				}*/

	NiagaraComponentFallingOil->DeactivateImmediate();
	//NiagaraComponentFallingOil->ResetSystem();

	NiagaraComponentFallingOil->DestroyComponent();

	if (NiagaraSystemFallingOil)
	{
		//UE_LOG(LogTrap, Warning, TEXT("FallingOil asset loaded successfully"));

		//NiagaraComponentFallingOil->SetAsset(NiagaraSystemFallingOil);

		NiagaraComponentFallingOil = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraSystemFallingOil, FallingOilNiagaraAttachPointOffset,
			NAME_None,
			FVector(0.f), FRotator(0.f),
			EAttachLocation::Type::KeepRelativeOffset, false, false);

		if (NiagaraComponentFallingOil)
		{
			NiagaraComponentFallingOil->OnSystemFinished.AddDynamic(this, &AOilBagTrap::OnNiagaraSystemFinished);
		}
	}

	/*NiagaraComponentFallingOil->SetPaused(true);

	if (NiagaraComponentFallingOil->IsPaused())
	{
		UE_LOG(LogTrap, Warning, TEXT("NiagaraComponentFallingOil->IsPaused true"));
	}*/

	/*if (HasAuthority())
	{
		UE_LOG(LogTrap, Warning, TEXT("NiagaraComponentFallingOil IsActive: %s"), NiagaraComponentFallingOil->IsActive() ? TEXT("True") : TEXT("False"));
	}*/
}

void AOilBagTrap::SpreadingOilCallback()
{
	/*if (HasAuthority())
				{
					UE_LOG(LogTrap, Warning, TEXT("NiagaraComponentSpreadingOil Deactivate"));
				}*/

				//NiagaraComponentSpreadingOil->Deactivate();
	NiagaraComponentSpreadingOil->DeactivateImmediate();
	//NiagaraComponentSpreadingOil->ResetSystem();

	NiagaraComponentSpreadingOil->DestroyComponent();

	if (NiagaraSystemSpreadingOil)
	{
		//UE_LOG(LogTrap, Warning, TEXT("FallingOil asset loaded successfully"));
		//NiagaraComponentFallingOil->SetAsset(NiagaraSystemFallingOil);

		NiagaraComponentSpreadingOil = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraSystemSpreadingOil, SpreadingOilNiagaraAttachPointOffset,
			NAME_None,
			FVector(0.f), FRotator(0.f),
			EAttachLocation::Type::KeepRelativeOffset, false, false);

		if (NiagaraComponentSpreadingOil)
		{
			NiagaraComponentSpreadingOil->OnSystemFinished.AddDynamic(this, &AOilBagTrap::OnNiagaraSystemFinished);
		}
	}

	/*if (HasAuthority())
	{
		UE_LOG(LogTrap, Warning, TEXT("NiagaraComponentSpreadingOil IsActive: %s"), NiagaraComponentSpreadingOil->IsActive() ? TEXT("True") : TEXT("False"));
	}*/
}

void AOilBagTrap::WreckageActivation()
{
	bWreckage = true;
	Mesh->SetHiddenInGame(false);

	ECollisionResponse BlockResponse = ECollisionResponse::ECR_Block;
	Trigger->SetCollisionResponseToChannel(ECC_GameTraceChannel11, BlockResponse); // DemolishReInstall
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, BlockResponse); // DemolishReInstall
	Trigger->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore); // Arrow
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore); // Arrow

	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Ignore);

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Ignore); // TurretProjectile

	ChangeWreckageMaterials(Mesh, RingMesh, RopeMesh);
	ChangeWreckageMapMarker();
}

void AOilBagTrap::ReInstall()
{
	bWreckage = false;
	bCanActivate = true;
	ChangeOriginalMaterial();
	ChangeAliveMapMarker();

	Trigger->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Arrow
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Arrow

	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Block);

	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	Trigger->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Overlap); // TurretProjectile

	Mesh->SetWorldTransform(ReInstallTransform);
}

void AOilBagTrap::Demolish()
{
	if (bWreckage)
	{
		return;
	}

	AActor* GC = GetWorld()->SpawnActor<AActor>(OilGC, GetActorTransform());
	if (GC)
	{
		GC->SetReplicates(true);
		GC->SetLifeSpan(5);
	}
}


void AOilBagTrap::MulticastRPCActivateEffect_Implementation()
{
	OnTrapActivatedBroadcast();

	//EffectSystem->Activate();
	//Mesh->SetHiddenInGame(true);

	//UE_LOG(LogTrap, Warning, TEXT("MulticastRPCActivateEffect_Implementation"));

	ECollisionResponse IgnoreResponse = ECollisionResponse::ECR_Ignore;
	Trigger->SetCollisionResponseToChannel(ECC_GameTraceChannel11, IgnoreResponse); // DemolishReInstall
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, IgnoreResponse); // DemolishReInstall
	Trigger->SetCollisionResponseToChannel(ECC_GameTraceChannel2, IgnoreResponse); // Arrow
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, IgnoreResponse); // Arrow

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AOilBagTrap::HiddenCallback, 2.2, false);

	// 오일 나이아가라 활성화
	if (IsValid(NiagaraComponentFallingOil))
	{
		//UE_LOG(LogTrap, Warning, TEXT("IsValid(NiagaraComponentFallingOil) true"));
		NiagaraComponentFallingOil->Activate();

		FTimerHandle FallingOilTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AOilBagTrap::FallingOilCallback, 5, false);
	}

	PlaySound();
}

void AOilBagTrap::MulticastRPCSetCollisionResponseOfSimulating_Implementation()
{
	OnTrapActivatedBroadcast();

	ECollisionResponse IgnoreResponse = ECollisionResponse::ECR_Ignore;
	Trigger->SetCollisionResponseToChannel(ECC_GameTraceChannel11, IgnoreResponse); // DemolishReInstall
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, IgnoreResponse); // DemolishReInstall
	Mesh->SetCollisionResponseToChannel(ECC_Camera, IgnoreResponse);
}

void AOilBagTrap::MulticastRPCSetHiddenMeshOfVelocity_Implementation()
{
	Mesh->SetHiddenInGame(true);
}

void AOilBagTrap::MulticastRPCSpreadingOil_Implementation(FVector Location)
{
	// 오일 나이아가라 활성화
	if (IsValid(NiagaraComponentSpreadingOil))
	{
		NiagaraComponentSpreadingOil->SetWorldLocation(Location);
		NiagaraComponentSpreadingOil->Activate();

		FTimerHandle SpreadingOilTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(SpreadingOilTimerHandle, this, &AOilBagTrap::SpreadingOilCallback, 6, false);
	}

	PlaySound();
}
