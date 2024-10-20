// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/BearTrap.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/TrapperPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "TrapperProject.h"
#include "Bow/Arrow.h"
#include "Animation/TrapAnimInstance.h"
#include "DamageType/TrapStunDamageType.h"
#include "Framework/TrapperGameMode.h"
#include "Ping/MarkerComponent/MapMarkerComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Bow/Arrow.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Character/TrapperPlayerMovementComponent.h"
#include "DamageType/DamageTypeStun.h"


ABearTrap::ABearTrap()
{
	MapMarkerwpqkfwpqkf->TrapType = EMapTrapType::BearAlive;

	Mesh->SetIsReplicated(true);

	CentralMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CentralMesh"));
	LeftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftMesh"));
	RightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightMesh"));
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	MagneticTrigger1 = CreateDefaultSubobject<UBoxComponent>(TEXT("MagneticTrigger1"));
	MagneticTrigger2 = CreateDefaultSubobject<UBoxComponent>(TEXT("MagneticTrigger2"));


	CentralMesh->SetupAttachment(Mesh);
	LeftMesh->SetupAttachment(Mesh);
	RightMesh->SetupAttachment(Mesh);
	SkeletalMesh->SetupAttachment(Mesh);
	MagneticTrigger1->SetupAttachment(Mesh);
	MagneticTrigger2->SetupAttachment(Mesh);

	Damage = 100.f;
}

void ABearTrap::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ABearTrap::OnOverlapBeginCharacter);
	Trigger->OnComponentEndOverlap.AddDynamic(this, &ABearTrap::OnOverlapEndCharacter);

	MagneticTrigger1->OnComponentHit.AddDynamic(this, &ABearTrap::OnHitMagneticArrow);
	MagneticTrigger2->OnComponentHit.AddDynamic(this, &ABearTrap::OnHitMagneticArrow);

	OriginalMaterialFirstElement = RightMesh->GetMaterial(0);
	OriginalMaterialSecondElement = RightMesh->GetMaterial(1);
}

void ABearTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void ABearTrap::Demolish()
{
	if (bWreckage)
	{
		return;
	}

	AActor* GC = GetWorld()->SpawnActor<AActor>(BearGC, GetActorTransform());
	if (GC)
	{
		GC->SetReplicates(true);
		GC->SetLifeSpan(5);
	}
}

void ABearTrap::ReInstall()
{
	//UE_LOG(LogTrap, Warning, TEXT("ReInstall"));

	bCanAttack = true;
	FirstMagneticTriggerActivatingPlayer = nullptr;
	SecondMagneticTriggerActivatingPlayer = nullptr;
	bWreckage = false;
	bReActivated = true;

	MagneticTrigger1->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Arrow
	MagneticTrigger2->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Arrow

	CentralMesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	LeftMesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	RightMesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);

	CentralMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Overlap); // TurretProjectile
	LeftMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Overlap); // TurretProjectile
	RightMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Overlap); // TurretProjectile
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Overlap); // TurretProjectile

	SkeletalMesh->SetPosition(0.0f);

	ChangeOriginalMaterial();
	ChangeAliveMapMarker();
}

void ABearTrap::MagneticTriggerControl(bool bIsOn)
{
	if (HasAuthority())
	{
		MulticastRPCMagneticTriggerControl(bIsOn);
	}
	else
	{
		ServerRPCMagneticTriggerControl(bIsOn);
	}
}

void ABearTrap::DestroyHandle()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ClearTriggerTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearTriggerTimerHandle);
	}
	if (GetWorld()->GetTimerManager().IsTimerActive(ReActivatedTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ReActivatedTimerHandle);
	}

	Destroy();
}

void ABearTrap::OnAttackedByBoss()
{
	if (bWreckage)
	{
		return;
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(ClearTriggerTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearTriggerTimerHandle);
	}

	if (HasAuthority())
	{
		if (BearGC)
		{
			SetActorEnableCollision(false);
			SetActorHiddenInGame(true);
			SetLifeSpan(3);

			MulticastRPCSpawnGC();
		}
	}
}

void ABearTrap::ServerRPCMagneticTriggerControl_Implementation(bool bIsOn)
{
	MulticastRPCMagneticTriggerControl(bIsOn);
}

void ABearTrap::MulticastRPCMagneticTriggerControl_Implementation(bool bIsOn)
{
	if (bIsOn)
	{
		MagneticTrigger1->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Arrow
		MagneticTrigger2->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Block); // Arrow
	}
	else
	{
		MagneticTrigger1->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore); // Arrow
		MagneticTrigger2->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore); // Arrow
	}
}

void ABearTrap::OnOverlapBeginCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	if (!HasAuthority() || !bCanAttack)
	{
		return;
	}

	if (!IsValid(OtherActor))
	{
		return;
	}

	// ���Ϳ� �÷��̾�� �ϴ� TSet�� ��Ƶ�
	if (OtherActor->ActorHasTag("Player") || OtherActor->ActorHasTag("Monster"))
	{
		//UE_LOG(LogTrap, Warning, TEXT("ABearTrap::OnOverlapBeginCharacter"));
		if (!OverlappingActors.Contains(OtherActor))
		{
			if (bSafeRangedFor)
			{
				//UE_LOG(LogTrap, Error, TEXT("BearTrap : bSafeRangedFor true -> Add Actor "));
				SafeAddArray.Add(OtherActor);
			}
			else
			{
				OverlappingActors.Add(OtherActor);
			}
		}
	}
}

void ABearTrap::OnOverlapEndCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority() || !bCanAttack)
	{
		return;
	}

	// ������ ������ ���� ���Ϳ��� ��� ��� �����̳ʿ��� ����
	if (OtherActor && OverlappingActors.Contains(OtherActor))
	{
		if (bSafeRangedFor)
		{
			//UE_LOG(LogTrap, Error, TEXT("BearTrap : bSafeRangedFor true -> Remove Actor "));
			SafeRemoveArray.Add(OtherActor);
		}
		else
		{
			OverlappingActors.Remove(OtherActor);
		}
	}
}

void ABearTrap::OnHitMagneticArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//UE_LOG(LogTrap, Warning, TEXT("OnOverlapBeginMagneticArrow"));

	if (!HasAuthority())
	{
		return;
	}

	if (OtherActor && OtherActor->ActorHasTag("MagneticArrow"))
	{
		//UE_LOG(LogTrap, Warning, TEXT("MagneticArrow"));

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

		if (Arrow)
		{
			//UE_LOG(LogTrap, Warning, TEXT("true"));

			bFirstActivatedTriggerIsServerPlayer = false;
			if (Arrow->IsSpawnedBy1P())
			{
				//UE_LOG(LogTrap, Warning, TEXT("1P NiagaraSystem"));
				bFirstActivatedTriggerIsServerPlayer = true;
			}
			else
			{
				//UE_LOG(LogTrap, Warning, TEXT("2P NiagaraSystem"));
				bFirstActivatedTriggerIsServerPlayer = false;
			}

			MulticastRPCSpawnNiagaraSystem(Hit.ImpactPoint, bFirstActivatedTriggerIsServerPlayer);
		}

		AimFeedback(OtherActor);

		//if (OverlappedComponent == FirstMagneticTrigger)
		if (HitComp == MagneticTrigger2)
		{

			// ���� ����� �ߵ����״ٸ� ����
			if (OwnerActor == FirstMagneticTriggerActivatingPlayer)
			{
				ClearTriggerTimer();
				return;
			}

			// Ʈ���Ÿ� �ߵ���Ų �÷��̾� ����
			FirstMagneticTriggerActivatingPlayer = OwnerActor;

			// ���׸��� ����
			if (Cast<ATrapperPlayer>(OwnerActor)->IsLocallyControlled())
			{
				//UE_LOG(LogTrap, Warning, TEXT("FirstMagneticTrigger RightMesh Server"));
				MulticastRPCChangeMaterial(ChangeMeshDirection::Right, ChangeMeshMaterial::Server);
			}
			else
			{
				//UE_LOG(LogTrap, Warning, TEXT("FirstMagneticTrigger RightMesh Client"));
				MulticastRPCChangeMaterial(ChangeMeshDirection::Right, ChangeMeshMaterial::Client);
			}

			// �ݴ��� �޽� �ߵ��� �÷��̾� üũ 
			// ���� �ߵ��� �޽��� ��������̶�� Ʈ���� �� ���׸��� ����
			if (OwnerActor == SecondMagneticTriggerActivatingPlayer)
			{
				SecondMagneticTriggerActivatingPlayer = nullptr;
				MulticastRPCChangeMaterial(ChangeMeshDirection::Left, ChangeMeshMaterial::Original);
			}

			//UE_LOG(LogTrap, Warning, TEXT("FirstMagneticTrigger"));
			ClearTriggerTimer();
		}

		else if (HitComp == MagneticTrigger1)
		{
			// ���� ����� �ߵ����״ٸ� ����
			if (OwnerActor == SecondMagneticTriggerActivatingPlayer)
			{
				ClearTriggerTimer();
				return;
			}

			// Ʈ���Ÿ� �ߵ���Ų �÷��̾� ����
			SecondMagneticTriggerActivatingPlayer = OwnerActor;

			// ���׸��� ����
			if (Cast<ATrapperPlayer>(OwnerActor)->IsLocallyControlled())
			{
				//UE_LOG(LogTrap, Warning, TEXT("SecondMagneticTrigger LeftMesh Server"));
				MulticastRPCChangeMaterial(ChangeMeshDirection::Left, ChangeMeshMaterial::Server);
			}
			else
			{
				//UE_LOG(LogTrap, Warning, TEXT("SecondMagneticTrigger LeftMesh Client"));
				MulticastRPCChangeMaterial(ChangeMeshDirection::Left, ChangeMeshMaterial::Client);
			}

			// �ݴ��� �޽� �ߵ��� �÷��̾� üũ 
			// ���� �ߵ��� �޽��� ��������̶�� Ʈ���� �� ���׸��� ����
			if (OwnerActor == FirstMagneticTriggerActivatingPlayer)
			{
				FirstMagneticTriggerActivatingPlayer = nullptr;

				MulticastRPCChangeMaterial(ChangeMeshDirection::Right, ChangeMeshMaterial::Original);
			}

			//UE_LOG(LogTrap, Warning, TEXT("SecondMagneticTrigger"));
			ClearTriggerTimer();
		}

		if (IsValid(FirstMagneticTriggerActivatingPlayer) && IsValid(SecondMagneticTriggerActivatingPlayer))
		{
			ATrapperPlayer* Player = Cast<ATrapperPlayer>(OtherActor->GetOwner());
			if (Player)
			{
				TriggeredByController = Player->GetController();
			}

			TrapperGameState->AddScore(EScoreType::TrapScoreAll, 100);

			ActivateTrap();
		}
	}
}

void ABearTrap::ActivateTrap()
{
	GetWorld()->GetGameState<ATrapperGameState>()->OnQuestExecute.Broadcast(11, true);

	bSafeRangedFor = true;

	for (AActor* Actor : OverlappingActors)
	{
		if (IsValid(Actor))
		{
			//if (Actor->ActorHasTag("Player"))
			//{
			//	//UE_LOG(LogTrap, Warning, TEXT("Player"));
			//	ACharacter* Character = Cast<ACharacter>(Actor);
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

			UClass* DamageTypeClass = nullptr;
			if (Actor->ActorHasTag("Boss"))
			{
				DamageTypeClass = UDamageType::StaticClass();
			}
			else
			{
				DamageTypeClass = UDamageTypeStun::StaticClass();
			}

			float FinalDamage = Damage * DamageRate;
			UGameplayStatics::ApplyDamage(Actor, FinalDamage, TriggeredByController.Get(), this, DamageTypeClass);
		}
	}

	CheckAndResetKillCountCollaboration(this);

	bSafeRangedFor = false;

	for (const auto& SafeAddActor : SafeAddArray)
	{
		//UE_LOG(LogTrap, Error, TEXT("BearTrap : SafeAddArray"));

		if (!OverlappingActors.Contains(SafeAddActor))
		{
			OverlappingActors.Add(SafeAddActor);
		}
	}
	SafeAddArray.Empty();

	for (const auto& SafeRemoveActor : SafeRemoveArray)
	{
		//UE_LOG(LogTrap, Error, TEXT("BearTrap : SafeRemoveArray"));

		if (OverlappingActors.Contains(SafeRemoveActor))
		{
			OverlappingActors.Remove(SafeRemoveActor);
		}
	}
	SafeRemoveArray.Empty();

	bCanAttack = false;
	MulticastRPCActivateEffect();
}

void ABearTrap::ClearTriggerTimer()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ClearTriggerTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearTriggerTimerHandle);
	}

	GetWorld()->GetTimerManager().SetTimer(ClearTriggerTimerHandle, this, &ABearTrap::ClearTriggerTimerCallback, 5.f, false);
}

void ABearTrap::WreckageActivation()
{
	bWreckage = true;

	GetWorld()->GetTimerManager().ClearTimer(ClearTriggerTimerHandle);
	//Destroy();
	SkeletalMesh->SetHiddenInGame(true);
	CentralMesh->SetHiddenInGame(false);
	LeftMesh->SetHiddenInGame(false);
	RightMesh->SetHiddenInGame(false);
	Mesh->SetHiddenInGame(false);
	ECollisionResponse BlockResponse = ECollisionResponse::ECR_Block;
	CentralMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, BlockResponse); // DemolishReInstall
	LeftMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, BlockResponse); // DemolishReInstall
	RightMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, BlockResponse); // DemolishReInstall
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, BlockResponse); // DemolishReInstall

	CentralMesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Ignore);
	LeftMesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Ignore);
	RightMesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Ignore);

	CentralMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Ignore); // TurretProjectile
	LeftMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Ignore); // TurretProjectile
	RightMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Ignore); // TurretProjectile
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Ignore); // TurretProjectile

	MagneticTrigger1->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore); // Arrow
	MagneticTrigger2->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore); // Arrow

	ChangeWreckageMaterials(LeftMesh, RightMesh, Mesh, CentralMesh);
	ChangeWreckageMapMarker();
}

void ABearTrap::ChangeOriginalMaterial()
{
	CentralMesh->SetMaterial(0, OriginalMaterialFirstElement);
	Mesh->SetMaterial(0, OriginalMaterialFirstElement);
	LeftMesh->SetMaterial(0, OriginalMaterialFirstElement);
	LeftMesh->SetMaterial(1, OriginalMaterialSecondElement);
	RightMesh->SetMaterial(0, OriginalMaterialFirstElement);
	RightMesh->SetMaterial(1, OriginalMaterialSecondElement);
}

void ABearTrap::PlayMagneticSound()
{
	if (MagneticSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, MagneticSound, GetActorLocation());
	}
}

void ABearTrap::ClearTriggerTimerCallback()
{
	//UE_LOG(LogTrap, Warning, TEXT("ClearTriggerTimer"));

	ATrapperGameState* GameState = GetWorld()->GetGameState<ATrapperGameState>();

	if (bFirstActivatedTriggerIsServerPlayer)
	{
		if (GameState && GameState->CurrentGameProgress == EGameProgress::Tutorial)
		{
			UE_LOG(LogTrap, Warning, TEXT("507"));
			GameState->OnEventExecute.Broadcast(507);
		}
	}
	else
	{
		if (GameState && GameState->CurrentGameProgress == EGameProgress::Tutorial)
		{
			UE_LOG(LogTrap, Warning, TEXT("508"));
			GameState->OnEventExecute.Broadcast(508);
		}
	}

	bReActivated = false;
	GetWorld()->GetTimerManager().SetTimer(ReActivatedTimerHandle, this, &ABearTrap::ReActivatedCallback,3, false);

	FirstMagneticTriggerActivatingPlayer = nullptr;
	SecondMagneticTriggerActivatingPlayer = nullptr;
	MulticastRPCChangeMaterial(ChangeMeshDirection::All, ChangeMeshMaterial::Original);
}

void ABearTrap::ClearTutorialTrap()
{
	GetWorld()->GetTimerManager().ClearTimer(ClearTriggerTimerHandle);
	Destroy();
}

void ABearTrap::ReActivatedCallback()
{
	bReActivated = true;
}

void ABearTrap::MulticastRPCSpawnGC_Implementation()
{
	SetActorEnableCollision(false);

	AActor* GC = GetWorld()->SpawnActor<AActor>(BearGC, GetActorTransform());
	if (GC)
	{
		GC->SetLifeSpan(10);
	}
}

void ABearTrap::MulticastRPCActivateEffect_Implementation()
{
	OnTrapActivatedBroadcast();

	/*UTrapAnimInstance* AnimInstance = Cast<UTrapAnimInstance>(SkeletalMesh->GetAnimInstance());
	if (AnimInstance)
	{
		SkeletalMesh->SetHiddenInGame(false);
		AnimInstance->SetActive(true);
	}*/

	SkeletalMesh->SetHiddenInGame(false);
	SkeletalMesh->Play(false);

	LeftMesh->SetHiddenInGame(true);
	RightMesh->SetHiddenInGame(true);
	Mesh->SetHiddenInGame(true);

	ECollisionResponse IgnoreResponse = ECollisionResponse::ECR_Ignore;
	LeftMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, IgnoreResponse); // DemolishReInstall
	RightMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, IgnoreResponse); // DemolishReInstall
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, IgnoreResponse); // DemolishReInstall

	if (ActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivateSound, GetActorLocation());
	}

	if (!bTutorialTrap)
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ABearTrap::WreckageActivation, 3, false);
	}
	else
	{
		if (HasAuthority())
		{
			// 튜토리얼 트랩
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ABearTrap::ClearTutorialTrap, 3, false);
		}
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ActivateNiagara,
		GetActorLocation(),
		FRotator::ZeroRotator,
		FVector(1.0f),
		true,
		true,
		ENCPoolMethod::None,
		true
	);
}

void ABearTrap::MulticastRPCChangeMaterial_Implementation(ChangeMeshDirection Direction, ChangeMeshMaterial Material)
{
	UMaterialInterface* ChangeMaterial = nullptr;
	UMaterialInterface* SecondChangeMaterial = nullptr;
	bool bIsOriginal = false;

	switch (Material)
	{
	case ChangeMeshMaterial::Server:
		ChangeMaterial = MaterialRed;
		break;
	case ChangeMeshMaterial::Client:
		ChangeMaterial = MaterialBlue;
		break;
	case ChangeMeshMaterial::Original:
		ChangeMaterial = OriginalMaterialFirstElement;
		SecondChangeMaterial = OriginalMaterialSecondElement;
		bIsOriginal = true;
		break;
	}

	switch (Direction)
	{
	case ChangeMeshDirection::Left:
		LeftMesh->SetMaterial(0, ChangeMaterial);

		if (!bIsOriginal)
		{
			LeftMesh->SetMaterial(1, ChangeMaterial);
		}
		else
		{
			LeftMesh->SetMaterial(1, SecondChangeMaterial);
		}
		break;
	case ChangeMeshDirection::Right:
		RightMesh->SetMaterial(0, ChangeMaterial);
		if (!bIsOriginal)
		{
			RightMesh->SetMaterial(1, ChangeMaterial);
		}
		else
		{
			RightMesh->SetMaterial(1, SecondChangeMaterial);
		}
		break;
	case ChangeMeshDirection::All:
		LeftMesh->SetMaterial(0, ChangeMaterial);
		LeftMesh->SetMaterial(1, SecondChangeMaterial);
		RightMesh->SetMaterial(0, ChangeMaterial);
		RightMesh->SetMaterial(1, SecondChangeMaterial);
		break;
	}
}

void ABearTrap::MulticastRPCSpawnNiagaraSystem_Implementation(const FVector& ImpactPoint, bool IsServer)
{
	//UE_LOG(LogTrap, Warning, TEXT("MulticastRPCSpawnNiagaraSystem_Implementation"));
	UNiagaraSystem* NiagaraSystem;
	if (IsServer)
	{
		NiagaraSystem = RedNiagara;
	}
	else
	{
		NiagaraSystem = BlueNiagara;
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
