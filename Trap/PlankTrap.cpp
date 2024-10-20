// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/PlankTrap.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/TrapperPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Trap/TrabSnabEnviroment/TrapSnabEnviromentBase.h"
#include "TrapperProject.h"
#include "DamageType/TrapStunDamageType.h"
#include "Ping/MarkerComponent/MapMarkerComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Bow/Arrow.h"
#include "Components/AudioComponent.h"
#include "Character/TrapperPlayerMovementComponent.h"
#include "DamageType/DamageTypeStun.h"
#include "Framework/TrapperGameState.h"


APlankTrap::APlankTrap()
{
	Trigger->SetupAttachment(DummyRoot);

	RopeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RopeMesh"));
	RopeMesh->SetupAttachment(DummyRoot);
	RingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMesh"));
	RingMesh->SetupAttachment(DummyRoot);

	MapMarkerwpqkfwpqkf->TrapType = EMapTrapType::PlankAlive;

	bIsSimulate = false;
	//DestroyTrapTime = 5.f;
	StopDamageOfVelocity = 0.f;
	CheckPlankVelocityTime = 0.2f;

	Damage = 80.f;

	Tags.Add(FName("PlankTrap"));
}

void APlankTrap::BeginPlay()
{
	Super::BeginPlay();

	ReInstallTransform = Mesh->GetComponentTransform();

	Trigger->OnComponentHit.AddDynamic(this, &APlankTrap::OnHitArrow);
	Mesh->OnComponentHit.AddDynamic(this, &APlankTrap::OnHitArrow);
	Mesh->OnComponentHit.AddDynamic(this, &APlankTrap::OnHit);

	/*FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			FVector Center = GetActorLocation();
			DrawDebugSphere(GetWorld(), Center, 1500, 16, FColor::Red, false, 0.1f, 0, 2);
		}, 0.1f, true);*/
}

void APlankTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void APlankTrap::Demolish()
{
	if (bWreckage)
	{
		return;
	}

	AActor* GC = GetWorld()->SpawnActor<AActor>(PlankGC, GetActorTransform());
	if (GC)
	{
		GC->SetReplicates(true);
		GC->SetLifeSpan(5);
	}
}

void APlankTrap::ReInstall()
{
	bPlayHitSound = false;
	bWreckage = false;
	bCanAttack = true;
	bActivateNiagara = false;

	OnHitActors.Empty();

	Mesh->SetSimulatePhysics(false);
	Mesh->SetEnableGravity(false);

	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Block); //DemolishReInstall
	Trigger->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Block); //DemolishReInstall
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block); //Arrow
	Trigger->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block); //Arrow
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);
	Trigger->SetCollisionResponseToChannel(ECC_Visibility, ECollisionResponse::ECR_Block);

	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, ECR_Overlap); // TurretProjectile

	Mesh->SetWorldTransform(ReInstallTransform);
	ChangeOriginalMaterial();
	ChangeAliveMapMarker();
}

void APlankTrap::OnHitArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("OnHitArrow"));

	// ȭ�쿡 ������ ���� �������Բ� ���� �ùķ��̼� On
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

		if (!bCanAttack)
		{
			return;
		}

		AActor* OwnerActor = OtherActor->GetOwner();
		if (!OwnerActor)
		{
			return;
		}

		if (HitComp == Mesh)
		{
			return;
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


		ATrapperPlayer* Player = Cast<ATrapperPlayer>(OwnerActor);
		if (Player)
		{
			TriggeredByController = Player->GetController();
		}

		// ���� �ǵ��
		AimFeedback(OtherActor);

		// �ùķ��̼� On
		if (!Mesh->IsSimulatingPhysics())
		{
			bCanAttack = false;
			Mesh->SetSimulatePhysics(true);
			Mesh->SetEnableGravity(true);
			Mesh->SetPhysicsLinearVelocity(FVector(0, 0, -600), true);
			MulticastRPCSetCollisionResponseOfSimulating();
		}

		// �������� �浹 üũ �� ������ ����
		if (HasAuthority())
		{
			bIsSimulate = true;

			//// ���� ȯ�� ������ Ʈ�� ��ġ ���� �缳��
			//if (IsValid(Owner))
			//{
			//	Cast<ATrapSnabEnviromentBase>(Owner)->SetInstalledTrap(false);
			//	Owner = nullptr;
			//}

			GetWorld()->GetTimerManager().SetTimer(CheckPlankVelocityTimerHandle, this, &APlankTrap::CheckPlankVelocity, CheckPlankVelocityTime, true);
			//GetWorld()->GetTimerManager().SetTimer(DestroyTrapTimerHandle, this, &APlankTrap::DestroyTrap, DestroyTrapTime, false);
		}
	}
}

void APlankTrap::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || !bIsSimulate)
	{
		return;
	}

	if (!bActivateNiagara)
	{
		bActivateNiagara = true;
		MulticastRPCActivateNiagara(Hit.ImpactPoint);
	}

	//UE_LOG(LogTrap, Warning, TEXT("OnHit"));

	/*if (Mesh->GetLinearDamping() != 0.01)
	{
		Mesh->SetLinearDamping(0.01);
	}*/

	// ���Ϳ� �÷��̾�� �������� �޴´�.
	if (OtherActor && (OtherActor->ActorHasTag("Monster") || OtherActor->ActorHasTag("Player")))
	{
		//UE_LOG(LogTrap, Warning, TEXT("Actor Name : %s"), *OtherActor->GetName());

		// �̹� �������� ���� ���ʹ� ��ŵ
		if (OtherActor && OnHitActors.Contains(OtherActor))
		{
			return;
		}

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

		UClass* DamageTypeClass = nullptr;
		if (OtherActor->ActorHasTag("Boss"))
		{
			DamageTypeClass = UDamageType::StaticClass();
		}
		else
		{
			DamageTypeClass = UDamageTypeStun::StaticClass();
		}

		float FinalDamage = Damage * DamageRate;
		UGameplayStatics::ApplyDamage(OtherActor, FinalDamage, TriggeredByController.Get(), this, DamageTypeClass);

		// Ÿ���� ���� ���ʹ� �����̳ʿ� �߰�
		OnHitActors.Add(OtherActor);
	}
}

void APlankTrap::CheckPlankVelocity()
{
	// �޽��� �ӷ��� StopDamageOfVelocity ���� ������ ������ off

	//UE_LOG(LogTrap, Warning, TEXT("Mesh->GetComponentVelocity().Size() : %f"), Mesh->GetComponentVelocity().Size());

	if (Mesh->GetComponentVelocity().Size() < 200)
	{
		bIsSimulate = false;
		CheckAndResetKillCount(TriggeredByController.Get(), this);

		MulticastRPCSetCollisionResponseOfVelocity();
	}

	if (Mesh->GetComponentVelocity().Size() <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(CheckPlankVelocityTimerHandle);
		Mesh->SetSimulatePhysics(false);
		Mesh->SetEnableGravity(false);

		MulticastRPCWreckActivation();
	}
}

void APlankTrap::DestroyTrap()
{
	GetWorld()->GetTimerManager().ClearTimer(DestroyTrapTimerHandle);

	Destroy();
}

void APlankTrap::WreckageActivation()
{
	bWreckage = true;

	ECollisionResponse IgnoreResponse = ECollisionResponse::ECR_Ignore;
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, IgnoreResponse);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Block); //DemolishReInstall
	Trigger->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Block); //DemolishReInstall
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, IgnoreResponse); //Arrow
	Trigger->SetCollisionResponseToChannel(ECC_GameTraceChannel2, IgnoreResponse); //Arrow

	Mesh->SetCollisionResponseToChannel(ECC_Visibility, IgnoreResponse);
	Trigger->SetCollisionResponseToChannel(ECC_Visibility, IgnoreResponse);

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel14, IgnoreResponse); // TurretProjectile

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	ChangeWreckageMaterials(Mesh, RingMesh, RopeMesh);
	ChangeWreckageMapMarker();
}

void APlankTrap::ChangeOriginalMaterial()
{
	RingMesh->SetMaterial(0, RingAndRopeOriginalMaterialFirstElement);
	RopeMesh->SetMaterial(0, RingAndRopeOriginalMaterialFirstElement);
	Mesh->SetMaterial(0, MeshOriginalMaterialFirstElement);
}

void APlankTrap::PlaySound()
{
	if (ActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivateSound, GetActorLocation());
	}
}


void APlankTrap::MulticastRPCWreckActivation_Implementation()
{
	WreckageActivation();
}

void APlankTrap::MulticastRPCSetCollisionResponseOfVelocity_Implementation()
{
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void APlankTrap::MulticastRPCSetCollisionResponseOfSimulating_Implementation()
{
	OnTrapActivatedBroadcast();

	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Ignore); //DemolishReInstall
	Trigger->SetCollisionResponseToChannel(ECC_GameTraceChannel11, ECR_Ignore); //DemolishReInstall

	PlaySound();
}

void APlankTrap::MulticastRPCActivateNiagara_Implementation(FVector ImpactPoint)
{
	ImpactPoint.Z += 20.f;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ActivateNiagara,
		ImpactPoint,
		FRotator::ZeroRotator,
		FVector(1.0f),
		true,
		true,
		ENCPoolMethod::None,
		true
	);
}
