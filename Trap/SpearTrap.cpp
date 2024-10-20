// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/SpearTrap.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/TrapperPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "TrapperProject.h"
#include "Bow/Arrow.h"
//#include "Net/UnrealNetwork.h"
#include "Ping/MarkerComponent/MapMarkerComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/TrapperPlayerMovementComponent.h"
#include "DamageType/DamageTypeKnockBack.h"
#include "Framework/TrapperGameState.h"


ASpearTrap::ASpearTrap()
{
	PrimaryActorTick.bCanEverTick = true;

	NiagaraComponentConnectBlue = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponentConnectBlue"));
	NiagaraComponentConnectBlue->SetupAttachment(Mesh);
	NiagaraComponentConnectBlue->SetAutoActivate(false);

	NiagaraComponentConnectRed = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponentConnectRed"));
	NiagaraComponentConnectRed->SetupAttachment(Mesh);
	NiagaraComponentConnectRed->SetAutoActivate(false);

	SpearWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpearWall"));
	SpearWall->SetupAttachment(DummyRoot);
	SpearWall->ComponentTags.Add(FName("SpearWall"));

	NiagaraComponentChargeNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponentChargeNiagara"));
	NiagaraComponentChargeNiagara->SetupAttachment(Mesh);
	NiagaraComponentChargeNiagara->SetAutoActivate(false);

	MapMarkerSpearWall = CreateDefaultSubobject<UMapMarkerComponent>(TEXT("MapMarkerSpearWall"));
	AddOwnedComponent(MapMarkerSpearWall);

	MapMarkerwpqkfwpqkf->TrapType = EMapTrapType::SpearAlive;
	MapMarkerSpearWall->TrapType = EMapTrapType::SpearAlive;
	Damage = 100;
	Tags.Add(FName("SpearTrap"));
}

void ASpearTrap::BeginPlay()
{
	Super::BeginPlay();

	Mesh->OnComponentHit.AddDynamic(this, &ASpearTrap::OnHitMagneticArrow);
	SpearWall->OnComponentHit.AddDynamic(this, &ASpearTrap::OnHitMagneticArrow);
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ASpearTrap::OnOverlapBeginCharacter);

	SpearOriginalMaterial = Mesh->GetMaterial(0);
	SpearWallOriginalMaterial = SpearWall->GetMaterial(1);

	WreckageLocation = GetActorLocation();

	CurrentBlinkDuration = InitialBlinkDuration;
}

void ASpearTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ClearAllTimer();
}

void ASpearTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActiveTrap)
	{
		ThrownSpear();
	}
}

void ASpearTrap::Demolish()
{
	if (bWreckage)
	{
		return;
	}

	AActor* GC = GetWorld()->SpawnActor<AActor>(SpearGC, Mesh->GetComponentTransform());
	if (GC)
	{
		GC->SetReplicates(true);
		GC->SetLifeSpan(5);
	}

	AActor* WGC = GetWorld()->SpawnActor<AActor>(WallGC, SpearWall->GetComponentTransform());
	if (WGC)
	{
		WGC->SetReplicates(true);
		WGC->SetLifeSpan(5);
	}
}

void ASpearTrap::ReInstall()
{
	bWreckage = false;
	bCanAttack = true;
	SpearMagneticTriggerActivatingPlayer = nullptr;
	SpearWallMagneticTriggerActivatingPlayer = nullptr;
	IncreaseCount = 0;
	CurrentBlinkDuration = InitialBlinkDuration;
	bReActivated = true;
	DynamicMaterialInstance = nullptr;
	bFirstOnBlinkTimer = true;
	OnHitActors.Reset();

	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Arrow
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Overlap); // TurretProjectile

	SpearWall->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	SpearWall->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	SpearWall->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Arrow
	SpearWall->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	SpearWall->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Overlap); // TurretProjectile

	ChangeOriginalMaterial();
	ChangeAliveMapMarker();
	MapMarkerSpearWall->SetTrapType(EMapTrapType::SpearAlive);

	FVector SpearWallLocation = SpearWall->GetComponentLocation();
	FVector SpearLocation = Mesh->GetComponentLocation();
	FVector Offset = SpearWallLocation - SpearLocation;
	MapMarkerSpearWall->SetOffset(Offset);
}

void ASpearTrap::OnAttackedByBoss()
{
	if (bWreckage)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	if (HasAuthority())
	{
		if (SpearGC && WallGC)
		{
			SetActorEnableCollision(false);
			SetActorHiddenInGame(true);
			SetLifeSpan(3);

			MulticastRPCSpawnGC();
		}
	}
}

void ASpearTrap::SetMapMarkerSpearWallInitialLocation()
{
	MulticastRPCSetMapMarkerSpearWallInitialLocation();
}

void ASpearTrap::ConnectionSpearAndWall()
{
	MulticastRPCConnectionSpearAndWall();
}

void ASpearTrap::OnHitMagneticArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(OtherActor))
	{
		return;
	}

	if (OtherActor->ActorHasTag("MagneticArrow"))
	{
		AArrow* Arrow = Cast<AArrow>(OtherActor);
		if (IsValid(Arrow) && Arrow->IsActivated())
		{
			Arrow->DeactivateArrow();
		}

		if (!bReActivated)
		{
			return;
		}

		if (!bCanAttack)
		{
			return;
		}

		AActor* OwnerActor = OtherActor->GetOwner();
		if (!OwnerActor)
		{
			return;
		}

		bIsServer = false;
		if (Arrow)
		{
			if (Arrow->IsSpawnedBy1P())
			{
				bIsServer = true;
			}
			else
			{
				bIsServer = false;
			}

			MulticastRPCSpawnNiagaraSystem(bIsServer, Hit.ImpactPoint);
		}

		AimFeedback(OtherActor);

		bool bChangeSpearMaterial = false;
		bool bChangeWallMaterial = false;

		bool bIsHitSpear = false;
		bool bIsHitWall = false;

		bool bIncreaseCountClear = false;

		if (HitComp == Mesh)
		{
			if (OwnerActor == SpearMagneticTriggerActivatingPlayer)
			{
				if (IncreaseCount != 4)
				{
					++IncreaseCount;
				}
				if (IncreaseCount < 4)
				{
					MulticastRPCPlayChargeSound(IncreaseCount, true, bIsServer);
				}
				else
				{
					MulticastRPCPlayCompleteChargeSound(true, bIsServer);
				}
				MulticastRPCChangeSpearMaterial(bIsServer, IncreaseCount);
				//ClearMagneticTimer();

				return;
			}
			else
			{
				bIncreaseCountClear = true;
			}

			if (OwnerActor == SpearWallMagneticTriggerActivatingPlayer)
			{
				IncreaseCount = 0;
				SpearWallMagneticTriggerActivatingPlayer = nullptr;

				MulticastRPCChangeSpearWallOriginalMaterial();
			}

			bool bIsActivate = false;
			if (SpearWallMagneticTriggerActivatingPlayer)
			{
				bIsActivate = true;
			}

			MulticastRPCInitializeBlink(bIsServer, true, bIsActivate);

			bIsHitSpear = true;
			SpearMagneticTriggerActivatingPlayer = OwnerActor;
			bChangeSpearMaterial = true;

			ClearMagneticTimer();
		}
		else if (HitComp == SpearWall)
		{
			if (OwnerActor == SpearWallMagneticTriggerActivatingPlayer)
			{
				if (IncreaseCount != 4)
				{
					++IncreaseCount;
				}
				if (IncreaseCount < 4)
				{
					MulticastRPCPlayChargeSound(IncreaseCount, false, bIsServer);
				}
				else
				{
					MulticastRPCPlayCompleteChargeSound(false, bIsServer);
				}
				MulticastRPCChangeSpearWallMaterial(bIsServer, IncreaseCount);
				//ClearMagneticTimer();
				return;
			}
			else
			{
				bIncreaseCountClear = true;
				DynamicMaterialInstance = nullptr;
			}

			if (OwnerActor == SpearMagneticTriggerActivatingPlayer)
			{
				IncreaseCount = 0;
				SpearMagneticTriggerActivatingPlayer = nullptr;

				MulticastRPCChangeSpearOriginalMaterial();
			}

			bool bIsActivate = false;
			if (SpearMagneticTriggerActivatingPlayer)
			{
				bIsActivate = true;
			}

			MulticastRPCInitializeBlink(bIsServer, false, bIsActivate);

			bIsHitWall = true;
			SpearWallMagneticTriggerActivatingPlayer = OwnerActor;
			bChangeWallMaterial = true;

			ClearMagneticTimer();
		}

		if (IsValid(SpearMagneticTriggerActivatingPlayer) && IsValid(SpearWallMagneticTriggerActivatingPlayer))
		{
			ATrapperPlayer* Player = Cast<ATrapperPlayer>(OtherActor->GetOwner());
			if (Player)
			{
				TriggeredByController = Player->GetController();
			}

			TrapperGameState->AddScore(EScoreType::TrapScoreAll, 100);

			bCanAttack = false;
			MulticastRPCActivateTrap(IncreaseCount);
		}
		else
		{
			if (bIncreaseCountClear)
			{
				IncreaseCount = 0;
			}

			MulticastRPCSpawnConnectionNiagaraSystem(bIsServer, bIsHitSpear, bIsHitWall);

			if (bChangeSpearMaterial)
			{
				MulticastRPCChangeSpearMaterial(bIsServer, IncreaseCount);
			}
			else if (bChangeWallMaterial)
			{
				MulticastRPCChangeSpearWallMaterial(bIsServer, IncreaseCount);
			}
		}
	}
}

void ASpearTrap::OnOverlapBeginCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bIsActiveTrap)
	{
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("OnOverlapBeginCharacter"));

	if (OtherActor->ActorHasTag("Player") || OtherActor->ActorHasTag("Monster"))
	{
		if (OnHitActors.Contains(OtherActor))
		{
			return;
		}

		float FinalDamage = Damage + (IncreaseDamage * IncreaseCount);
		float FinalStrength = Strength + (IncreaseStrength * IncreaseCount);

		//UE_LOG(LogTrap, Warning, TEXT("IncreaseCount : %d"), IncreaseCount);
		//UE_LOG(LogTrap, Warning, TEXT("FinalDamage %f"), FinalDamage);
		//UE_LOG(LogTrap, Warning, TEXT("FinalStrength %f"), FinalStrength);

		//if (OtherActor->ActorHasTag("Player"))
		//{
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

		FinalDamage *= DamageRate;

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

		OnHitActors.Add(OtherActor);

		if (OtherActor->ActorHasTag("Boss"))
		{
			return;
		}

		FVector PlayerLocation = OtherActor->GetActorLocation();
		FVector SpearLocation = GetActorLocation();
		SpearLocation.Z = PlayerLocation.Z;

		FVector LaunchDirection = (OtherActor->GetActorUpVector() * 600) + ((PlayerLocation - SpearLocation).GetSafeNormal() * 1500);
		LaunchDirection = LaunchDirection.GetSafeNormal();

		//FVector LaunchDirection = (OtherActor->GetActorUpVector() * 300) + ((PlayerLocation - SpearLocation).GetSafeNormal() * 1500);

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

void ASpearTrap::ThrownSpear()
{
	FVector SpearLocation = Mesh->GetComponentLocation();
	FVector WallLocation = SpearWall->GetComponentLocation();

	float FinalSpeed = Speed + (IncreaseSpeed * IncreaseCount);
	//UE_LOG(LogTrap, Warning, TEXT("IncreaseCount : %d"), IncreaseCount);

	// 프레임당 이동 거리 계산
	FVector Direction = (WallLocation - SpearLocation).GetSafeNormal();
	FVector NewLocation = SpearLocation + Direction * FinalSpeed * GetWorld()->GetDeltaSeconds();

	// 목표 지점에 도달하거나 넘어섰는지 확인
	if (FVector::Dist(NewLocation, WallLocation) < 375.f)
	{
		bIsActiveTrap = false;
		SetWeckageOnTimer();

		Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		Trigger->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		CheckAndResetKillCountCollaboration(this);

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ActivateNiagara2,
			Mesh->GetComponentLocation(),
			FRotator(90.0f, 0.0f, 0.0f),
			FVector(1.0f),
			true,
			true,
			ENCPoolMethod::None,
			true
		);
	}

	Mesh->SetWorldLocation(NewLocation);
}

void ASpearTrap::ClearMagneticTimer()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ClearTriggerTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearTriggerTimerHandle);
	}

	//UE_LOG(LogTrap, Warning, TEXT("IncreaseCount : %d"), IncreaseCount);
	//UE_LOG(LogTrap, Warning, TEXT("Damage : %f"), Damage);

	GetWorld()->GetTimerManager().SetTimer(ClearTriggerTimerHandle, this, &ASpearTrap::ClearTriggerTimerCallback, 12, false);
}

void ASpearTrap::PlayMagneticSound()
{
	if (MagneticSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MagneticSound, GetActorLocation());
	}
}

void ASpearTrap::WreckageActivation()
{
	//UE_LOG(LogTrap, Warning, TEXT("WreckageActivation"));

	Mesh->SetWorldLocation(WreckageLocation);

	bWreckage = true;

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Block); // DemolishReInstall
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore); //Arrow
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Ignore); // TurretProjectile

	SpearWall->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Block); // DemolishReInstall
	SpearWall->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SpearWall->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore); //Arrow
	SpearWall->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SpearWall->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	SpearWall->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Ignore); // TurretProjectile

	ChangeWreckageMaterials(Mesh, SpearWall);
	ChangeWreckageMapMarker();
	MapMarkerSpearWall->SetTrapType(EMapTrapType::SpearWreckage);

	FVector SpearWallLocation = SpearWall->GetComponentLocation();
	FVector SpearLocation = Mesh->GetComponentLocation();
	FVector Offset = SpearWallLocation - SpearLocation;
	MapMarkerSpearWall->SetOffset(Offset);
}

void ASpearTrap::ChangeOriginalMaterial()
{
	Mesh->SetMaterial(0, SpearOriginalMaterial);
	SpearWall->SetMaterial(0, SpearWallOriginalMaterial);
}

void ASpearTrap::SetWeckageOnTimer()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ASpearTrap::SetWeckageOnTimerCallback, 3, false);
}

void ASpearTrap::PlayChargeSound(uint8 Count, bool IsSpear, bool IsServer)
{
	float SpawnRate = 0.f;
	USoundWave* Sound = nullptr;
	switch (Count)
	{
	case 1:
		Sound = ChargeSound1;
		SpawnRate = 5.f;
		break;
	case 2:
		Sound = ChargeSound2;
		SpawnRate = 10.f;
		break;
	case 3:
		Sound = ChargeSound3;
		SpawnRate = 20.f;
		break;
	default:
		UE_LOG(LogTrap, Warning, TEXT("Sound Count Error"));
	}

	FLinearColor Color;
	if (bIsServer)
	{
		Color = { 1.f,0.f,0.f,1.f };
	}
	else
	{
		Color = { 0.f,0.f,1.f,1.f };
	}

	FVector Location;
	if (IsSpear)
	{
		Location = Mesh->GetComponentLocation();
		Location = (FromSpearToWall.Vector() * 200.f) + Location;
	}
	else
	{
		Location = SpearWall->GetComponentLocation();
	}

	NiagaraComponentChargeNiagara->SetWorldLocation(Location);
	NiagaraComponentChargeNiagara->SetVariableLinearColor(FName("Color"), Color);
	NiagaraComponentChargeNiagara->SetVariableFloat(FName("SpawnRate"), SpawnRate);
	NiagaraComponentChargeNiagara->Activate(true);

	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
	}
}

void ASpearTrap::PlayCompleteChargeSound(bool IsSpear, bool IsServer)
{
	FLinearColor Color;
	if (IsServer)
	{
		Color = { 1.f,0.f,0.f,1.f };
	}
	else
	{
		Color = { 0.f,0.f,1.f,1.f };
	}

	FVector Location;
	if (IsSpear)
	{
		Location = Mesh->GetComponentLocation();
		Location = (FromSpearToWall.Vector() * 200.f) + Location;
	}
	else
	{
		Location = SpearWall->GetComponentLocation();
	}

	NiagaraComponentChargeNiagara->SetWorldLocation(Location);
	NiagaraComponentChargeNiagara->SetVariableLinearColor(FName("Color"), Color);
	NiagaraComponentChargeNiagara->SetVariableFloat(FName("SpawnRate"), 40);
	NiagaraComponentChargeNiagara->Activate(true);

	if (CompleteChargeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CompleteChargeSound, GetActorLocation());
	}
}

void ASpearTrap::PlayLaunchSound()
{
	if (ActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivateSound, GetActorLocation());
	}
}

void ASpearTrap::OnSpearBlinkTimer(bool IsServer)
{
	//UE_LOG(LogTrap, Warning, TEXT("OnSpearBlinkTimer"));
	if (bFirstOnBlinkTimer)
	{
		//UE_LOG(LogTrap, Warning, TEXT("bFirstOnBlinkTimer true"));
		bFirstOnBlinkTimer = false;

		bIsServer = IsServer;
		GetWorld()->GetTimerManager().SetTimer(BlinkTimerHandle, this, &ASpearTrap::BlinkTimerCallback, CurrentBlinkDuration, true);
		return;
	}

	Mesh->SetMaterial(0, SpearOriginalMaterial);

	bIsServer = IsServer;
	GetWorld()->GetTimerManager().SetTimer(BlinkChangeMaterialTimerHandle, this, &ASpearTrap::BlinkChangeMaterialTimerCallback, 0.15, false);
	GetWorld()->GetTimerManager().SetTimer(BlinkTimerHandle, this, &ASpearTrap::BlinkTimerCallback, CurrentBlinkDuration, true);
}

void ASpearTrap::OnWallBlinkTimer(bool IsServer)
{
	//UE_LOG(LogTrap, Warning, TEXT("OnWallBlinkTimer"));
	if (bFirstOnBlinkTimer)
	{
		//UE_LOG(LogTrap, Warning, TEXT("bFirstOnBlinkTimer true"));
		bFirstOnBlinkTimer = false;

		bIsServer = IsServer;
		GetWorld()->GetTimerManager().SetTimer(BlinkTimerHandle, this, &ASpearTrap::WallBlinkTimerCallback, CurrentBlinkDuration, true);
		return;
	}

	SpearWall->SetMaterial(0, SpearWallOriginalMaterial);

	bIsServer = IsServer;
	GetWorld()->GetTimerManager().SetTimer(BlinkChangeMaterialTimerHandle, this, &ASpearTrap::WallBlinkChangeMaterialTimerCallback, 0.15, false);
	GetWorld()->GetTimerManager().SetTimer(BlinkTimerHandle, this, &ASpearTrap::WallBlinkTimerCallback, CurrentBlinkDuration, true);
}

void ASpearTrap::UpdateDecreaseBlink()
{
	GetWorld()->GetTimerManager().SetTimer(UpdateDecreaseBlinkTimerHandle, this, &ASpearTrap::UpdateDecreaseBlinkTimerCallback, 1, true);
}

void ASpearTrap::ClearAllTimer()
{
	DynamicMaterialInstance = nullptr;

	if (GetWorld()->GetTimerManager().IsTimerActive(ClearTriggerTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearTriggerTimerHandle);
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(ReActivatedTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ReActivatedTimerHandle);
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(BlinkTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(BlinkTimerHandle);
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(UpdateDecreaseBlinkTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateDecreaseBlinkTimerHandle);
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(BlinkChangeMaterialTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(BlinkChangeMaterialTimerHandle);
	}
}

void ASpearTrap::ClearBlinkTimer()
{
	DynamicMaterialInstance = nullptr;

	if (GetWorld()->GetTimerManager().IsTimerActive(BlinkTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(BlinkTimerHandle);
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(UpdateDecreaseBlinkTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateDecreaseBlinkTimerHandle);
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(BlinkChangeMaterialTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(BlinkChangeMaterialTimerHandle);
	}
}

void ASpearTrap::OnBlinkTimer(bool IsServer, bool IsSpear)
{
	UpdateDecreaseBlink();

	if (IsSpear)
	{
		OnSpearBlinkTimer(IsServer);
	}
	else
	{
		OnWallBlinkTimer(IsServer);
	}
}

void ASpearTrap::ClearTriggerTimerCallback()
{
	SpearMagneticTriggerActivatingPlayer = nullptr;
	SpearWallMagneticTriggerActivatingPlayer = nullptr;
	IncreaseCount = 0;
	bReActivated = false;
	bFirstOnBlinkTimer = true;
	CurrentBlinkDuration = InitialBlinkDuration;

	MulticastRPCChangeAllMeshOriginalMaterial();

	GetWorld()->GetTimerManager().SetTimer(ReActivatedTimerHandle, this, &ASpearTrap::ReActivatedTimerCallback, 10, false);
}

void ASpearTrap::ReActivatedTimerCallback()
{
	bReActivated = true;
}

void ASpearTrap::SetWeckageOnTimerCallback()
{
	WreckageActivation();
}

void ASpearTrap::BlinkTimerCallback()
{
	OnSpearBlinkTimer(bIsServer);
}

void ASpearTrap::WallBlinkTimerCallback()
{
	OnWallBlinkTimer(bIsServer);
}

void ASpearTrap::BlinkChangeMaterialTimerCallback()
{
	if (DynamicMaterialInstance)
	{
		Mesh->SetMaterial(0, DynamicMaterialInstance);
	}
	else
	{
		if (bIsServer)
		{
			Mesh->SetMaterial(0, MaterialRed);
		}
		else
		{
			Mesh->SetMaterial(0, MaterialBlue);
		}
	}
}

void ASpearTrap::WallBlinkChangeMaterialTimerCallback()
{
	if (DynamicMaterialInstance)
	{
		SpearWall->SetMaterial(0, DynamicMaterialInstance);
	}
	else
	{
		if (bIsServer)
		{
			SpearWall->SetMaterial(0, MaterialRed);
		}
		else
		{
			SpearWall->SetMaterial(0, MaterialBlue);
		}
	}
}

void ASpearTrap::UpdateDecreaseBlinkTimerCallback()
{
	CurrentBlinkDuration = CurrentBlinkDuration - BlinkDecreaseRate;
	if (CurrentBlinkDuration < FinalBlinkDuration)
	{
		CurrentBlinkDuration = FinalBlinkDuration;
	}
}

void ASpearTrap::MulticastRPCSpawnGC_Implementation()
{
	SetActorEnableCollision(false);

	AActor* SGC = GetWorld()->SpawnActor<AActor>(SpearGC, Mesh->GetComponentTransform());
	if (SGC)
	{
		SGC->SetLifeSpan(10);
	}

	AActor* WGC = GetWorld()->SpawnActor<AActor>(WallGC, SpearWall->GetComponentTransform());
	if (WGC)
	{
		WGC->SetLifeSpan(10);
	}
}

void ASpearTrap::MulticastRPCSetMapMarkerSpearWallInitialLocation_Implementation()
{
	FVector SpearWallLocation = SpearWall->GetComponentLocation();
	FVector SpearLocation = Mesh->GetComponentLocation();
	FVector Offset = SpearWallLocation - SpearLocation;
	MapMarkerSpearWall->SetOffset(Offset);
}

void ASpearTrap::MulticastRPCSpawnNiagaraSystem_Implementation(bool IsServer, const FVector& ImpactPoint)
{
	UNiagaraSystem* NiagaraSystem;
	if (IsServer)
	{
		NiagaraSystem = MagneticRedNiagara;
	}
	else
	{
		NiagaraSystem = MagneticBlueNiagara;
	}

	//UE_LOG(LogTrap, Warning, TEXT("SpawnSystemAtLocation"));
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

	//PlayMagneticSound();
}

void ASpearTrap::MulticastRPCSpawnConnectionNiagaraSystem_Implementation(bool IsServer, bool IsHitSpear, bool IsHitWall)
{
	FVector MeshLocation = Mesh->GetComponentLocation();
	FVector WallLocation = SpearWall->GetComponentLocation();

	if (IsHitSpear)
	{
		if (IsServer)
		{
			NiagaraComponentConnectBlue->SetWorldLocation(WallLocation);
			NiagaraComponentConnectRed->SetWorldLocation(MeshLocation);

			NiagaraComponentConnectRed->SetWorldRotation(FromSpearToWall);
		}
		else
		{
			NiagaraComponentConnectBlue->SetWorldLocation(MeshLocation);
			NiagaraComponentConnectRed->SetWorldLocation(WallLocation);

			NiagaraComponentConnectBlue->SetWorldRotation(FromSpearToWall);
		}
	}
	else if (IsHitWall)
	{
		if (IsServer)
		{
			NiagaraComponentConnectBlue->SetWorldLocation(MeshLocation);
			NiagaraComponentConnectRed->SetWorldLocation(WallLocation);

			NiagaraComponentConnectRed->SetWorldRotation(FromWallToSpear);
		}
		else
		{
			NiagaraComponentConnectBlue->SetWorldLocation(WallLocation);
			NiagaraComponentConnectRed->SetWorldLocation(MeshLocation);

			NiagaraComponentConnectBlue->SetWorldRotation(FromWallToSpear);
		}
	}

	if (IsServer)
	{
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
}

void ASpearTrap::MulticastRPCChangeSpearMaterial_Implementation(bool IsServer, uint8 Count)
{
	//UE_LOG(LogTrap, Warning, TEXT("Count : %d"), Count);
	if (IsServer)
	{
		if (Count == 0)
		{
			Mesh->SetMaterial(0, MaterialRed);
		}
		else
		{
			DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialRed, this);
			if (DynamicMaterialInstance)
			{
				FLinearColor  LinearColor;
				DynamicMaterialInstance->GetVectorParameterValue(FName("Color"), LinearColor);
				LinearColor.R = LinearColor.R + (LinearColor.R * Count);
				DynamicMaterialInstance->SetVectorParameterValue(FName("Color"), LinearColor);

				float Opacity;
				DynamicMaterialInstance->GetScalarParameterValue(FName("Opacity"), Opacity);
				Opacity = Opacity + (OpacityIncrement * Count);
				DynamicMaterialInstance->SetScalarParameterValue(FName("Opacity"), Opacity);

				//UE_LOG(LogTrap, Warning, TEXT("LinearColor R : %f"), LinearColor.R);

				Mesh->SetMaterial(0, DynamicMaterialInstance);
			}
		}
	}
	else
	{
		if (Count == 0)
		{
			Mesh->SetMaterial(0, MaterialBlue);
		}
		else
		{
			DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialBlue, this);
			if (DynamicMaterialInstance)
			{
				FLinearColor  LinearColor;
				DynamicMaterialInstance->GetVectorParameterValue(FName("Color"), LinearColor);
				LinearColor.G = LinearColor.G + (LinearColor.G * Count);
				LinearColor.B = LinearColor.B + (LinearColor.B * Count);
				DynamicMaterialInstance->SetVectorParameterValue(FName("Color"), LinearColor);

				float Opacity;
				DynamicMaterialInstance->GetScalarParameterValue(FName("Opacity"), Opacity);
				Opacity = Opacity + (OpacityIncrement * Count);
				DynamicMaterialInstance->SetScalarParameterValue(FName("Opacity"), Opacity);

				Mesh->SetMaterial(0, DynamicMaterialInstance);
			}
		}
	}
}

void ASpearTrap::MulticastRPCChangeSpearWallMaterial_Implementation(bool IsServer, uint8 Count)
{
	if (IsServer)
	{
		if (Count == 0)
		{
			SpearWall->SetMaterial(0, MaterialRed);
		}
		else
		{
			DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialRed, this);
			if (DynamicMaterialInstance)
			{
				FLinearColor  LinearColor;
				DynamicMaterialInstance->GetVectorParameterValue(FName("Color"), LinearColor);
				LinearColor.R = LinearColor.R + (LinearColor.R * Count);
				DynamicMaterialInstance->SetVectorParameterValue(FName("Color"), LinearColor);

				float Opacity;
				DynamicMaterialInstance->GetScalarParameterValue(FName("Opacity"), Opacity);
				Opacity = Opacity + (OpacityIncrement * Count);
				DynamicMaterialInstance->SetScalarParameterValue(FName("Opacity"), Opacity);

				SpearWall->SetMaterial(0, DynamicMaterialInstance);
			}
		}
	}
	else
	{
		if (Count == 0)
		{
			SpearWall->SetMaterial(0, MaterialBlue);
		}
		else
		{
			DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialBlue, this);
			if (DynamicMaterialInstance)
			{
				FLinearColor  LinearColor;
				DynamicMaterialInstance->GetVectorParameterValue(FName("Color"), LinearColor);
				LinearColor.G = LinearColor.G + (LinearColor.G * Count);
				LinearColor.B = LinearColor.B + (LinearColor.B * Count);
				DynamicMaterialInstance->SetVectorParameterValue(FName("Color"), LinearColor);

				float Opacity;
				DynamicMaterialInstance->GetScalarParameterValue(FName("Opacity"), Opacity);
				Opacity = Opacity + (OpacityIncrement * Count);
				DynamicMaterialInstance->SetScalarParameterValue(FName("Opacity"), Opacity);

				SpearWall->SetMaterial(0, DynamicMaterialInstance);
			}
		}
	}
}

void ASpearTrap::MulticastRPCChangeSpearOriginalMaterial_Implementation()
{
	ClearBlinkTimer();

	CurrentBlinkDuration = InitialBlinkDuration;
	bFirstOnBlinkTimer = true;

	Mesh->SetMaterial(0, SpearOriginalMaterial);

	NiagaraComponentConnectBlue->DeactivateImmediate();
	NiagaraComponentConnectRed->DeactivateImmediate();
}

void ASpearTrap::MulticastRPCChangeSpearWallOriginalMaterial_Implementation()
{
	ClearBlinkTimer();

	CurrentBlinkDuration = InitialBlinkDuration;
	bFirstOnBlinkTimer = true;

	SpearWall->SetMaterial(0, SpearWallOriginalMaterial);

	NiagaraComponentConnectBlue->DeactivateImmediate();
	NiagaraComponentConnectRed->DeactivateImmediate();
}

void ASpearTrap::MulticastRPCChangeAllMeshOriginalMaterial_Implementation()
{
	ClearBlinkTimer();

	Mesh->SetMaterial(0, SpearOriginalMaterial);
	SpearWall->SetMaterial(0, SpearWallOriginalMaterial);

	NiagaraComponentConnectBlue->DeactivateImmediate();
	NiagaraComponentConnectRed->DeactivateImmediate();
	NiagaraComponentChargeNiagara->DeactivateImmediate();
}

void ASpearTrap::MulticastRPCActivateTrap_Implementation(uint8 Count)
{
	//UE_LOG(LogTrap, Warning, TEXT("MulticastRPCActivateTrap_Implementation"));
	ClearAllTimer();

	bIsActiveTrap = true;

	IncreaseCount = Count;

	NiagaraComponentConnectRed->DeactivateImmediate();
	NiagaraComponentConnectBlue->DeactivateImmediate();
	NiagaraComponentChargeNiagara->DeactivateImmediate();

	OnTrapActivatedBroadcast();

	Mesh->SetMaterial(0, SpearOriginalMaterial);
	SpearWall->SetMaterial(0, SpearWallOriginalMaterial);

	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Ignore); // DemolishReInstall
	SpearWall->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Ignore); // DemolishReInstall

	PlayLaunchSound();

	/*UNiagaraFunctionLibrary::SpawnSystemAttached(
		WindNiagara, RootComponent,
		NAME_None,
		FVector(0.f), FRotator(0.f),
		EAttachLocation::Type::KeepRelativeOffset, true, true);*/

	UNiagaraFunctionLibrary::SpawnSystemAttached(
		ActivateNiagara1,
		Mesh,
		NAME_None,
		FVector(),
		FRotator::ZeroRotator,
		EAttachLocation::Type::KeepRelativeOffset,
		true,
		true
	);
}

void ASpearTrap::MulticastRPCConnectionSpearAndWall_Implementation()
{
	FVector MeshLocation = Mesh->GetComponentLocation();
	FVector WallLocation = SpearWall->GetComponentLocation();

	FVector Direction = WallLocation - MeshLocation;
	FVector NormalizedDirection = Direction.GetSafeNormal();
	FromSpearToWall = NormalizedDirection.Rotation();
	FromWallToSpear = (-NormalizedDirection).Rotation();

	FVector2D BeamLength = FVector2D(50, abs(FVector::Distance(MeshLocation, WallLocation)));

	NiagaraComponentConnectBlue->SetVariableVec2(FName("BeamLength"), BeamLength);
	NiagaraComponentConnectRed->SetVariableVec2(FName("BeamLength"), BeamLength);
}

void ASpearTrap::MulticastRPCPlayChargeSound_Implementation(uint8 Count, bool IsSpear, bool IsServer)
{
	PlayChargeSound(Count, IsSpear, IsServer);
}

void ASpearTrap::MulticastRPCPlayCompleteChargeSound_Implementation(bool IsSpear, bool IsServer)
{
	PlayCompleteChargeSound(IsSpear, IsServer);
}

void ASpearTrap::MulticastRPCInitializeBlink_Implementation(bool IsServer, bool IsSpear, bool IsActivate)
{
	DynamicMaterialInstance = nullptr;

	CurrentBlinkDuration = InitialBlinkDuration;
	bFirstOnBlinkTimer = true;

	OnBlinkTimer(IsServer, IsSpear);

	FLinearColor Color;
	if (IsServer)
	{
		Color = { 1.f,0.f,0.f,1.f };
	}
	else
	{
		Color = { 0.f,0.f,1.f,1.f };
	}

	FVector Location;
	if (IsSpear)
	{
		Location = Mesh->GetComponentLocation();
		Location = (FromSpearToWall.Vector() * 200.f) + Location;
	}
	else
	{
		Location = SpearWall->GetComponentLocation();
	}

	NiagaraComponentChargeNiagara->SetWorldLocation(Location);
	NiagaraComponentChargeNiagara->SetVariableLinearColor(FName("Color"), Color);
	NiagaraComponentChargeNiagara->SetVariableFloat(FName("SpawnRate"), 2);
	NiagaraComponentChargeNiagara->Activate(true);

	if (IsActivate)
	{
		PlayMagneticSound();
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(this, ChargeSound0, GetActorLocation());
	}
}