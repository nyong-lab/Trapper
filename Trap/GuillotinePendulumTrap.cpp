// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/GuillotinePendulumTrap.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/TrapperPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "TrapperProject.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Trap/TrabSnabEnviroment/GuillotineTrapSnabEnviroment.h"
#include "Ping/MarkerComponent/MapMarkerComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Bow/Arrow.h"
#include "Components/AudioComponent.h"
#include "Character/TrapperPlayerMovementComponent.h"
#include "DamageType/DamageTypeKnockBack.h"
#include "Framework/TrapperGameState.h"
#include "Trap/TrabSnabEnviroment/TrapSnapTrigger.h"


AGuillotinePendulumTrap::AGuillotinePendulumTrap()
{
	MapMarkerwpqkfwpqkf->TrapType = EMapTrapType::None;

	PrimaryActorTick.bCanEverTick = true;

	Beam = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Beam"));
	Beam->SetupAttachment(DummyRoot);
	Panel1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel1"));
	Panel1->SetupAttachment(Beam);
	Panel2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel2"));
	Panel2->SetupAttachment(Beam);

	Mesh->SetupAttachment(Beam);

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(Mesh);

	NiagaraComponentSand = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponentSand"));
	NiagaraComponentSand->SetupAttachment(Mesh);
	NiagaraComponentSand->SetAutoActivate(false);

	/*ActivateNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ActivateNiagara"));
	ActivateNiagara->SetupAttachment(TriggerSphere);
	ActivateNiagara->SetAutoActivate(false);*/

	ActivateTrapTime = 0.f;
	ReEnableTime = 20.f;
	ReAttackTime = 1.5f;
	MaxRoll = 70.f;
	bMagneticTriggerActivated = false;
	bIsActiveTrap = false;
	bFirstMagneticTriggerDirection = true;
	bIsLerpingToTarget = false;
	bLerpCompleted = false;
	bMeshMagneticTrigger = false;
	bPanel1MagneticTrigger = false;
	bPanel2MagneticTrigger = false;
	bFirstRestrictionActivation = false;
	bSoundRight = false;
	bPlayMoveSound = false;
	Volume = 1.05;

	bSoundChangeHit2 = false;
	bSoundChangeHit1 = false;

	bNiagaraDeactivate = false;

	Damage = 150.f;

	Tags.Add(FName("GuillotinePendulumTrap"));
}

void AGuillotinePendulumTrap::BeginPlay()
{
	Super::BeginPlay();

	InitialMeshTransform = Mesh->GetComponentTransform();

	Mesh->OnComponentHit.AddDynamic(this, &AGuillotinePendulumTrap::OnHitArrow);
	Panel1->OnComponentHit.AddDynamic(this, &AGuillotinePendulumTrap::OnHitArrow);
	Panel2->OnComponentHit.AddDynamic(this, &AGuillotinePendulumTrap::OnHitArrow);
	Beam->OnComponentHit.AddDynamic(this, &AGuillotinePendulumTrap::OnHitArrow);

	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AGuillotinePendulumTrap::OnOverapCharacter);

	TrapPrevLocation = GetComponentByClass<UBoxComponent>()->GetComponentTransform().GetLocation();
}

void AGuillotinePendulumTrap::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void AGuillotinePendulumTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("AGuillotinePendulumTrap Tick"));
	if (bIsActiveTrap)
	{
		TickSwing();
	}
}

void AGuillotinePendulumTrap::Demolish()
{
	UE_LOG(LogTrap, Warning, TEXT("AGuillotinePendulumTrap::Demolish()"));
	if (ActorHasTag("Small"))
	{
		AActor* BGC = GetWorld()->SpawnActor<AActor>(BeamSmallGC, Beam->GetComponentTransform());
		if (BGC)
		{
			BGC->SetReplicates(true);
			BGC->SetLifeSpan(5);
		}

		AActor* MGC = GetWorld()->SpawnActor<AActor>(MeshSmallGC, Mesh->GetComponentTransform());
		if (MGC)
		{
			MGC->SetReplicates(true);
			MGC->SetLifeSpan(5);
		}
	}
	else
	{
		AActor* BGC = GetWorld()->SpawnActor<AActor>(BeamGC, Beam->GetComponentTransform());
		if (BGC)
		{
			BGC->SetReplicates(true);
			BGC->SetLifeSpan(5);
		}

		AActor* MGC = GetWorld()->SpawnActor<AActor>(MeshGC, Mesh->GetComponentTransform());
		if (MGC)
		{
			MGC->SetReplicates(true);
			MGC->SetLifeSpan(5);
		}
	}
}

void AGuillotinePendulumTrap::OnAttackedByBoss()
{
	UE_LOG(LogTrap, Warning, TEXT("AGuillotinePendulumTrap::OnAttackedByBoss"));
	if (GetWorld()->GetTimerManager().IsTimerActive(ClearTriggerTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearTriggerTimerHandle);
	}

	if (HasAuthority())
	{
		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
		Demolish();
		MulticastRPCSpawnGC();
	}
}

void AGuillotinePendulumTrap::SetActiveTrap(bool Value/*활성화 여부*/, bool FirstMagneticTriggerDirection /*첫 번째 마그네틱 트리거 방향인지*/)
{
	OnTrapActivatedBroadcast();

	NiagaraComponentSand->Activate(true);
	//ActivateNiagara->Activate(true);

	bSoundRight = false;
	bPlayMoveSound = false;
	Volume = 1.05;
	bNiagaraDeactivate = false;

	bSoundChangeHit2 = false;
	bSoundChangeHit1 = false;

	InitialDamage = Damage;

	//Mesh->SetCollisionProfileName(TEXT("NoCollision"));
	//Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	MeshMagneticTriggerActivatingPlayer = nullptr;
	MulticastRPCSetMeshMaterialOriginal();

	if (IsValid(Panel1MagneticTriggerActivatingPlayer))
	{
		Panel1MagneticTriggerActivatingPlayer = nullptr;
		MulticastRPCSetPanel1MaterialOriginal(true);
	}
	else
	{
		Panel2MagneticTriggerActivatingPlayer = nullptr;
		MulticastRPCSetPanel1MaterialOriginal(false);
	}

	// 이미 발동된 상태이므로 쿨타임 동안 재 발동 false
	bCanAttack = false;

	// 트랩 자성 화살 트리거 false
	bMagneticTriggerActivated = false;

	// 트랩 발동 누적 시간 초기화
	ActivateTrapTime = 0.f;

	// 트랩 활성화 (실질적 Tick 함수)
	bIsActiveTrap = Value;

	// 트랩 스윙 방향
	bFirstMagneticTriggerDirection = FirstMagneticTriggerDirection;

	// 진자 운동 시작 전 러프 
	bIsLerpingToTarget = true;

	// 함정 재발동 가능 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(ReEnableTimerHandle, this, &AGuillotinePendulumTrap::SetEnableCanAttack, ReEnableTime, false);
}

void AGuillotinePendulumTrap::SetEnableCanAttack()
{
	//UE_LOG(LogTrap, Warning, TEXT("SetEnableCanAttack"));
	bCanAttack = true;

	//UE_LOG(LogTrap, Warning, TEXT("Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);"));
	//Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void AGuillotinePendulumTrap::TickSwing()
{
	FVector CurrentLocation = GetComponentByClass<UBoxComponent>()->GetComponentTransform().GetLocation();
	//TrapPrevLocation = GetComponentByClass<UBoxComponent>()->GetComponentTransform().GetLocation();

	TrapFrameNormal = (CurrentLocation - TrapPrevLocation).GetSafeNormal();
	LaunchSpeed = FVector::Distance(CurrentLocation, TrapPrevLocation);
	//UE_LOG(LogTrap, Warning, TEXT("CurrentLocation : %s"), *CurrentLocation.ToString());
	//UE_LOG(LogTrap, Warning, TEXT("TrapPrevLocation : %s"), *TrapPrevLocation.ToString());
	//UE_LOG(LogTrap, Warning, TEXT("LaunchSpeed : %f"), LaunchSpeed);

	ActivateTrapTime += GetWorld()->GetDeltaSeconds();

	float NewRoll = 0.f;
	float SinValue = 0.f;
	float DampingFactor = 0.f;

	if (ActivateTrapTime > 50.f)
	{
		static float LastLerp = 0.f;
		LastLerp += GetWorld()->GetDeltaSeconds() * 0.2;
		NewRoll = FMath::Lerp(Mesh->GetComponentRotation().Roll, 0.f, LastLerp);

		if (LastLerp >= 1)
		{
			LastLerp = 0.f;
			bIsActiveTrap = false;
		}

		FRotator TrapRotator = Mesh->GetComponentRotation();
		TrapRotator.Roll = NewRoll;
		Mesh->SetWorldRotation(TrapRotator);
		//Mesh->SetRelativeRotation(TrapRotator);

		return;
	}

	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 수치 보는 용도 (레전드 괴물) ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

	//CalculatePendulum(NewRoll, SinValue, DampingFactor);

	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 위에서 본 수치를 토대로 값 넣어주기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

	// 타겟으로 러프
	if (bIsLerpingToTarget)
	{
		float DestinationRoll = MaxRoll;

		/// 조정 포인트 1
		// 첫 번째 마그네틱 트리거 방향이 아니라면 반대로 스윙
		if (!bFirstMagneticTriggerDirection)
		{
			DestinationRoll *= -1;
			NewRoll = FMath::Lerp(Mesh->GetComponentRotation().Roll, DestinationRoll, GetWorld()->GetDeltaSeconds() * 2400 / FMath::Pow(FMath::Abs(GetActorRotation().Roll + 70), 1.5));
		}
		else
		{
			NewRoll = FMath::Lerp(Mesh->GetComponentRotation().Roll, DestinationRoll, GetWorld()->GetDeltaSeconds() * 2400 / FMath::Pow(FMath::Abs(GetActorRotation().Roll - 70), 1.5));
		}

		/// 조정 포인트 2
		if (FMath::Abs(NewRoll) >= 67.3f)
		{
			NewRoll = (NewRoll > 0) ? 67.33f : -67.33f;
			bIsLerpingToTarget = false;
			bLerpCompleted = true;
		}
	}
	// 러프가 끝나면 최초 한번 발동
	else if (bLerpCompleted)
	{
		/// 조정 포인트 3 (위 수치 테스트에서 얻은 값 넣기)
		ActivateTrapTime = 0.777386;
		bLerpCompleted = false;

		CalculatePendulum(NewRoll, SinValue, DampingFactor);

		LastRoll = GetActorRotation().Roll;
	}
	// 본격적인 진자 운동 
	else
	{
		CalculatePendulum(NewRoll, SinValue, DampingFactor);
		CalculateDamage(NewRoll);
		LastRoll = NewRoll;
	}
	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

	/*FRotator TrapRotator = GetActorRotation();
	TrapRotator.Roll = NewRoll;
	SetActorRotation(TrapRotator);*/

	FRotator TrapRotator = Mesh->GetComponentRotation();
	TrapRotator.Roll = NewRoll;
	Mesh->SetWorldRotation(TrapRotator);

	TrapPrevLocation = CurrentLocation;

	//UE_LOG(LogTrap, Warning, TEXT("TrapRotator.Roll : %f"), TrapRotator.Roll);
	//UE_LOG(LogTrap, Warning, TEXT("ActivateTrapTime : %f"), ActivateTrapTime);
	//UE_LOG(LogTrap, Warning, TEXT("Damage : %f"), Damage);

	CurrentRollForDetermineSwingDirection = NewRoll;
	//UE_LOG(LogTrap, Warning, TEXT("Current Roll : %f"), CurrentRollForDetermineSwingDirection);
	//UE_LOG(LogTrap, Warning, TEXT("Previous Roll : %f"), PreviousRollForDetermineSwingDirection);


	/// 나이아가라
	float ResultValue = FMath::Abs(CurrentRollForDetermineSwingDirection - PreviousRollForDetermineSwingDirection);

	/// 사운드 

	if (Volume == 0)
	{
		//UE_LOG(LogTrap, Warning, TEXT("Volume 0 return"));
		return;
	}

	// 스윙 방향 판단
	if (CurrentRollForDetermineSwingDirection > PreviousRollForDetermineSwingDirection)
	{
		if (bSoundRight == false)
		{
			//UE_LOG(LogTrap, Warning, TEXT("A"));
			bSoundRight = true;
			bPlayMoveSound = true;
			bCheckKillCount = true;
		}
	}
	else
	{
		if (bSoundRight == true)
		{
			//UE_LOG(LogTrap, Warning, TEXT("B"));
			bSoundRight = false;
			bPlayMoveSound = true;
			bCheckKillCount = true;
		}
	}

	if (HasAuthority())
	{
		if (bCheckKillCount)
		{
			//UE_LOG(LogTrap, Warning, TEXT("Guillotine CheckAndResetKillCountCollaboration"));
			bCheckKillCount = false;
			CheckAndResetKillCountCollaboration(this);
		}
	}

	PreviousRollForDetermineSwingDirection = CurrentRollForDetermineSwingDirection;

	if (bPlayMoveSound)
	{
		FVector2D NiagaraScale = { 90,30 };
		NiagaraScale.X -= ActivateTrapTime * 3;
		NiagaraScale.Y -= ActivateTrapTime;

		if (NiagaraScale.X > 6 && NiagaraScale.Y > 2)
		{
			//UE_LOG(LogTrap, Warning, TEXT("NiagaraSacle : %s"), *NiagaraScale.ToString());

			NiagaraComponentSand->SetVariableVec2(FName("ValueTest"), NiagaraScale);
			NiagaraComponentSand->Activate(true);
		}
		else
		{
			if (NiagaraComponentSand->IsActive())
			{
				NiagaraComponentSand->DeactivateImmediate();
			}
		}

		bPlayMoveSound = false;

		Volume -= 0.05;
		//UE_LOG(LogTrap, Warning, TEXT("Volume %f"), Volume);

		//MoveAudioComponent->SetVolumeMultiplier(Volume);

		if (bSoundRight)
		{
			CurrentMoveSound = MoveSound1;
		}
		else
		{
			CurrentMoveSound = MoveSound2;
		}

		if (CurrentMoveSound)
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AGuillotinePendulumTrap::MoveSoundCallback, 0.4, false);
		}
	}

	/*UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ActivateNiagara1,
		TriggerSphere->GetComponentLocation(),
		TriggerSphere->GetComponentRotation(),
		FVector(1.0f),
		true,
		true,
		ENCPoolMethod::None,
		true
	);*/
}

void AGuillotinePendulumTrap::AllMaterialBlue()
{
	//UE_LOG(LogTrap, Warning, TEXT("AllMaterialBlue"));
	if (Mesh)
	{
		//UE_LOG(LogTrap, Warning, TEXT("AllMaterialBlue Mesh"));
		Mesh->SetMaterial(0, MaterialBlue);
		Mesh->SetMaterial(1, MaterialBlue);
		Mesh->SetMaterial(2, MaterialBlue);
	}
	if (Beam)
	{
		Beam->SetMaterial(0, MaterialBlue);
		Beam->SetMaterial(1, MaterialBlue);
	}
	if (Panel1)
	{
		Panel1->SetMaterial(0, MaterialBlue);
		Panel1->SetMaterial(1, MaterialBlue);
	}
	if (Panel2)
	{
		Panel2->SetMaterial(0, MaterialBlue);
		Panel2->SetMaterial(1, MaterialBlue);
	}

	/*if (HasAuthority())
	{
		UE_LOG(LogTrap, Warning, TEXT("Server AllMaterialBlue"));
	}
	else
	{
		UE_LOG(LogTrap, Warning, TEXT("Client AllMaterialBlue"));
	}*/
}

void AGuillotinePendulumTrap::AllMaterialRed()
{
	if (Mesh)
	{
		Mesh->SetMaterial(0, MaterialRed);
		Mesh->SetMaterial(1, MaterialRed);
		Mesh->SetMaterial(2, MaterialRed);
	}
	if (Beam)
	{
		Beam->SetMaterial(0, MaterialRed);
		Beam->SetMaterial(1, MaterialRed);
	}
	if (Panel1)
	{
		Panel1->SetMaterial(0, MaterialRed);
		Panel1->SetMaterial(1, MaterialRed);
	}
	if (Panel2)
	{
		Panel2->SetMaterial(0, MaterialRed);
		Panel2->SetMaterial(1, MaterialRed);
	}
}

bool AGuillotinePendulumTrap::IsCanInstall() const
{
	if (OverlappingTraps.Num() == 0)
	{
		return true;
	}

	//UE_LOG(LogTrap, Warning, TEXT("OverlappingActors.Num() : %d"), OverlappingActors.Num());

	return false;
}

void AGuillotinePendulumTrap::SetAllMaterialOriginal()
{
	MulticastRPCSetAllMaterialOriginal();
}

void AGuillotinePendulumTrap::OnHitArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT(" AGuillotinePendulumTrap::OnOverlapBegin"));

	// 화살에 맞으면 땅에 떨어지게끔 물리 시뮬레이션 On
	if (OtherActor && OtherActor->ActorHasTag("MagneticArrow"))
	{
		AArrow* Arrow = Cast<AArrow>(OtherActor);
		if (IsValid(Arrow) && Arrow->IsActivated())
		{
			Arrow->DeactivateArrow();
		}

		if (!bCanAttack)
		{
			return;
		}

		//UE_LOG(LogTrap, Warning, TEXT(" MagneticArrow"));
		AActor* OwnerActor = OtherActor->GetOwner();
		if (!OwnerActor)
		{
			return;
		}

		bool bIsServer = false;
		if (IsValid(Arrow))
		{
			//UE_LOG(LogTrap, Warning, TEXT("true"));

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
		}

		if (HitComp == Mesh)
		{
			MulticastRPCSpawnNiagaraSystem(bIsServer, Hit.ImpactPoint);

			if (OwnerActor == MeshMagneticTriggerActivatingPlayer)
			{
				ClearTriggerTimer();

				return;
			}

			MeshMagneticTriggerActivatingPlayer = OwnerActor;

			AimFeedback(OtherActor);

			if (Arrow->IsSpawnedBy1P())
			{
				MulticastRPCSetMeshMaterialBlue(false);
			}
			else
			{
				MulticastRPCSetMeshMaterialBlue(true);
			}

			if (OwnerActor == Panel1MagneticTriggerActivatingPlayer)
			{
				Panel1MagneticTriggerActivatingPlayer = nullptr;
				MulticastRPCSetPanel1MaterialOriginal(true);
			}
			else if (OwnerActor == Panel2MagneticTriggerActivatingPlayer)
			{
				Panel2MagneticTriggerActivatingPlayer = nullptr;
				MulticastRPCSetPanel1MaterialOriginal(false);
			}

			ClearTriggerTimer();
		}

		else if (HitComp == Panel1)
		{
			if (OwnerActor == Panel1MagneticTriggerActivatingPlayer)
			{
				ClearTriggerTimer();

				return;
			}

			if (!bFirstRestrictionActivation)
			{
				return;
			}

			MulticastRPCSpawnNiagaraSystem(bIsServer, Hit.ImpactPoint);

			//UE_LOG(LogTrap, Warning, TEXT("OverlappedComponent == Panel"));

			Panel1MagneticTriggerActivatingPlayer = OwnerActor;

			AimFeedback(OtherActor);

			if (Arrow->IsSpawnedBy1P())
			{
				MulticastRPCSetPanel1MaterialBlue(false);
			}
			else
			{
				MulticastRPCSetPanel1MaterialBlue(true);
			}

			if (IsValid(Panel2MagneticTriggerActivatingPlayer))
			{
				Panel2MagneticTriggerActivatingPlayer = nullptr;
				MulticastRPCSetPanel1MaterialOriginal(false);
			}
			else if (OwnerActor == MeshMagneticTriggerActivatingPlayer)
			{
				MeshMagneticTriggerActivatingPlayer = nullptr;
				MulticastRPCSetMeshMaterialOriginal();
			}

			ClearTriggerTimer();
		}

		else if (HitComp == Panel2)
		{
			MulticastRPCSpawnNiagaraSystem(bIsServer, Hit.ImpactPoint);

			if (OwnerActor == Panel2MagneticTriggerActivatingPlayer)
			{
				ClearTriggerTimer();
				return;
			}

			//UE_LOG(LogTrap, Warning, TEXT("OverlappedComponent == Panel2"));

			Panel2MagneticTriggerActivatingPlayer = OwnerActor;

			AimFeedback(OtherActor);

			if (Arrow->IsSpawnedBy1P())
			{
				MulticastRPCSetPanel2MaterialBlue(false);
			}
			else
			{
				MulticastRPCSetPanel2MaterialBlue(true);
			}

			if (IsValid(Panel1MagneticTriggerActivatingPlayer))
			{
				Panel1MagneticTriggerActivatingPlayer = nullptr;
				MulticastRPCSetPanel1MaterialOriginal(true);
			}
			else if (OwnerActor == MeshMagneticTriggerActivatingPlayer)
			{
				MeshMagneticTriggerActivatingPlayer = nullptr;
				MulticastRPCSetMeshMaterialOriginal();
			}

			ClearTriggerTimer();
		}

		if (IsValid(MeshMagneticTriggerActivatingPlayer) &&
			(IsValid(Panel1MagneticTriggerActivatingPlayer) || IsValid(Panel2MagneticTriggerActivatingPlayer)))
		{
			//UE_LOG(LogTrap, Warning, TEXT("Activate Trap"));

			ATrapperPlayer* Player = Cast<ATrapperPlayer>(OtherActor->GetOwner());
			if (Player)
			{
				TriggeredByController = Player->GetController();
			}

			if (!bFirstRestrictionActivation)
			{
				bFirstRestrictionActivation = true;
			}

			TrapperGameState->AddScore(EScoreType::TrapScoreAll, 100);

			MulticastRPCSetTrapActive(true, IsValid(Panel1MagneticTriggerActivatingPlayer));
		}

		//// 에임 피드백
		//AimFeedback(OtherActor);

		//// 자성화살 트리거 활성화
		//bMagneticTriggerActivated = true;

		//// 어떤 플레이어가 활성화 시켯는지 저장
		////SetActivatingPlayer(OtherActor);
		//MulticastRPCSetActivingPlayer(OtherActor);

		//// 스냅 환경 액터 존재 유무 체크
		//if (IsValid(Owner))
		//{
		//	AGuillotineTrapSnabEnviroment* SnabEnviroment = Cast<AGuillotineTrapSnabEnviroment>(Owner);
		//	if (IsValid(SnabEnviroment))
		//	{
		//		// 환경 액터에 자성화살 트리거 체크 
		//		if (SnabEnviroment->IsMagneticTriggerActivated())
		//		{
		//			// 트랩 활성화 -> Tick에서 함정 로테이션 변동 시작
		//			//SetTrapActive(true, SnabEnviroment->IsFirstMagneticTriggerActivated(), SnabEnviroment);
		//			MulticastRPCSetTrapActive(true, SnabEnviroment->IsFirstMagneticTriggerActivated());
		//		}
		//	}
		//}
	}
}

void AGuillotinePendulumTrap::OnOverapCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	if (!HasAuthority())
	{
		return;
	}

	/*if (bIsActiveTrap)
	{
		MulticastRPCActivateEffectt(SweepHitResult.ImpactPoint);
	}*/

	//UE_LOG(LogTrap, Warning, TEXT("TriggerSphere OnOverapCharacter"));

	// 몬스터와 플레이어는 데미지를 받는다.
	if (bIsActiveTrap && OtherActor && (OtherActor->ActorHasTag("Player") || OtherActor->ActorHasTag("Monster")))
	{
		// 이미 데미지를 받은 액터는 스킵
		if (OtherActor && OnHitActors.Contains(OtherActor))
		{
			return;
		}

		// Sound
		MulticastRPCPlayHitSound();

		//UE_LOG(LogTrap, Warning, TEXT("OtherActor Name : %s"), *OtherActor->GetName());

		// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

		FVector LaunchNormal = SweepHitResult.Normal;
		FVector HitNormal;
		FVector CalculateLocation;

		if (LaunchNormal.Size() == 0)
		{
			CalculateLocation = OtherActor->GetActorLocation() - GetComponentByClass<UBoxComponent>()->GetComponentTransform().GetLocation();
			LaunchNormal = CalculateLocation.GetSafeNormal();
		}

		HitNormal = LaunchNormal * -1;

		float Power = TrapFrameNormal.Dot(HitNormal);
		Power += 1;

		FVector LaunchDirection;
		FVector PlayerHitDirection;

		if (Power > 0 && Power < 1)
		{
			//UE_LOG(LogTrap, Warning, TEXT("Power > 0 && Power < 1"));
			PlayerHitDirection = LaunchNormal * Power * LaunchSpeed * 60;
		}
		else
		{
			//UE_LOG(LogTrap, Warning, TEXT("Power > 1"));
			PlayerHitDirection = LaunchNormal * Power * LaunchSpeed * 90;
		}

		//넉백 처리
		LaunchDirection = PlayerHitDirection * -1;

		// 무조건 공중으론 날리자
		if (LaunchDirection.Z < 0)
		{
			LaunchDirection.Z *= -1;
		}

		if (InitialDamage > 30)
		{
			if (0 < LaunchDirection.X && LaunchDirection.X < 500)
			{
				LaunchDirection.X += 500;
			}
			else if (0 > LaunchDirection.X && LaunchDirection.X > -500)
			{
				LaunchDirection.X -= 500;
			}

			if (0 < LaunchDirection.Y && LaunchDirection.Y < 500)
			{
				LaunchDirection.Y += 500;
			}
			else if (0 > LaunchDirection.Y && LaunchDirection.Y > -500)
			{
				LaunchDirection.Y -= 500;
			}

			if (0 < LaunchDirection.Z && LaunchDirection.Z < 500)
			{
				LaunchDirection.Z += 500;
			}
			else if (0 > LaunchDirection.Z && LaunchDirection.Z < -500)
			{
				LaunchDirection.Z -= 500;
			}

			if (OtherActor->GetVelocity().Size() == 0)
			{
				LaunchDirection *= 2;
			}
		}
		else
		{
			//UE_LOG(LogTrap, Warning, TEXT("Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);"));
			//Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}

		if ((CalculateLocation.X > 0 && LaunchDirection.X < 0) || (CalculateLocation.X < 0 && LaunchDirection.X > 0))
		{
			/*UE_LOG(LogTrap, Warning, TEXT("-------------XXXXXXXXX-----------------------"));
			UE_LOG(LogTrap, Warning, TEXT("CalculateLocation : %s"), *CalculateLocation.ToString());
			UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());*/

			LaunchDirection.X *= -1;

			/*UE_LOG(LogTrap, Warning, TEXT("-------------------------------------"));
			UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());
			UE_LOG(LogTrap, Warning, TEXT("-------------------------------------"));*/
		}
		if ((CalculateLocation.Y > 0 && LaunchDirection.Y < 0) || CalculateLocation.Y < 0 && LaunchDirection.Y > 0)
		{
			/*UE_LOG(LogTrap, Warning, TEXT("-------------YYYYYYYYY-----------------------"));
			UE_LOG(LogTrap, Warning, TEXT("CalculateLocation : %s"), *CalculateLocation.ToString());
			UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());*/

			LaunchDirection.Y *= -1;
			/*UE_LOG(LogTrap, Warning, TEXT("-------------------------------------"));
			UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());
			UE_LOG(LogTrap, Warning, TEXT("-------------------------------------"));*/
		}

		if (LaunchDirection.X > 700)
		{
			LaunchDirection.X = 700;
		}
		else if (LaunchDirection.X < -700)
		{
			LaunchDirection.X = -700;
		}

		if (LaunchDirection.Y > 700)
		{
			LaunchDirection.Y = 700;
		}
		else if (LaunchDirection.Y < -700)
		{
			LaunchDirection.Y = -700;
		}

		if (LaunchDirection.Z > 700)
		{
			LaunchDirection.Z = 700;
		}
		else if (LaunchDirection.Z < -700)
		{
			LaunchDirection.Z = -700;
		}

		if (!OtherActor->ActorHasTag("Boss"))
		{
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

		/*	UE_LOG(LogTrap, Warning, TEXT("HitNormal : %s"), *HitNormal.ToString());
			UE_LOG(LogTrap, Warning, TEXT("Power : %f"), Power);
			UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());
			UE_LOG(LogTrap, Warning, TEXT("LaunchSize : %f"), LaunchDirection.Size());
			UE_LOG(LogTrap, Warning, TEXT("-------------------------------------"));*/

			//DrawDebugLine(GetWorld(), SweepHitResult.ImpactPoint, SweepHitResult.ImpactPoint + LaunchDirection, FColor::Red, true);

			// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

			//UE_LOG(LogTrap, Warning, TEXT("Actor Name : %s"), *OtherActor->GetName());

		float FinalDamage = InitialDamage * DamageRate;

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

		// 타격을 입은 액터는 N초후 다시 데미지를 입히기
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AGuillotinePendulumTrap::ReAttackTimeCallback, ReAttackTime, false);
		HitActorTimers.Add(OtherActor, TimerHandle);
		//GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, OtherActor]()
		//	{
		//		//UE_LOG(LogTrap, Warning, TEXT("ReAttackTimer On"));
		//		OutHitActor(OtherActor);
		//	}, ReAttackTime, false
		//);

		// 이펙트 활성화
		//MulticastRPCActivateEffect();
	}
}

void AGuillotinePendulumTrap::CalculatePendulum(float& NewRoll, float& SinValue, float& DampingFactor)
{
	// Sin(Time * 2 ) 의미 : 시간을 두 배 빠르게 흐르게 하겠다.
	// -> sin 그래프 한 바퀴가 아마 3.14? 초인데 3.14 / 2 를 한 것 (아마도)
	// 
	// * 60 의미 : sin 그래프가 1 ~ -1  (y축? yaw? Value?) 를 의미하는데 60 ~ -60 을 하기 위함
	// 
	// time * ? 의미 : 그래프 value ? yaw? 값이 점점 줄어드는걸 원하는데 time을 그대로 넣었더니 너무 급하게 줄어들어서 0.x 값을 곱해서 천천히 줄어들게 만듬
	// Yaw = Value / (Time *0.1);
	// y = sin(x) * e^(-x*0.05)

	SinValue = FMath::Sin(ActivateTrapTime * 2) * 70;
	DampingFactor = FMath::Exp(-ActivateTrapTime * 0.05);

	// 첫 번째 마그네틱 트리거 방향이 아니라면 반대로 스윙
	if (!bFirstMagneticTriggerDirection)
	{
		SinValue *= -1;
	}

	NewRoll = SinValue * DampingFactor;
}

void AGuillotinePendulumTrap::CalculateDamage(const float& NewRoll)
{
	if (ActivateTrapTime > 10.f)
	{
		InitialDamage = FMath::Abs((NewRoll - LastRoll) / GetWorld()->GetDeltaSeconds());

		if (!bSoundChangeHit2)
		{
			bSoundChangeHit2 = true;
			CurrentHitSound = HitSound2;
			//UE_LOG(LogTrap, Warning, TEXT("bSoundChangeHit2 true"));
		}

		if (ActivateTrapTime > 40.f)
		{
			InitialDamage = 10.f;
		}
		else if (ActivateTrapTime > 30.f)
		{
			InitialDamage = 20.f;
		}
		else
		{
			if (ActivateTrapTime > 20.f)
			{
				if (!bSoundChangeHit1)
				{
					bSoundChangeHit1 = true;
					CurrentHitSound = HitSound1;
					//UE_LOG(LogTrap, Warning, TEXT("bSoundChangeHit1 true"));
				}
			}

			if (InitialDamage < 30.f)
			{
				InitialDamage = 30.f;
			}
		}
	}

	//UE_LOG(LogTrap, Warning, TEXT("Damage : %f"), Damage);
}

void AGuillotinePendulumTrap::OutHitActor(AActor* OtherActor)
{
	if (OtherActor && OnHitActors.Contains(OtherActor))
	{
		FTimerHandle* TimerHandle = HitActorTimers.Find(OtherActor);
		if (TimerHandle)
		{
			HitActorTimers.Remove(OtherActor);
		}

		OnHitActors.Remove(OtherActor);
	}
}

void AGuillotinePendulumTrap::SetActivatingPlayer(AActor* OtherActor)
{
	AActor* OwnerActor = OtherActor->GetOwner();
	if (!OwnerActor)
	{
		return;
	}
	ActivatingPlayer = OwnerActor;
}

void AGuillotinePendulumTrap::SetMeshMaterialBlue(bool Value)
{
	if (Value)
	{
		Mesh->SetMaterial(0, MaterialBlue);
		Mesh->SetMaterial(1, MaterialBlue);
		Mesh->SetMaterial(2, MaterialBlue);
	}
	else
	{
		Mesh->SetMaterial(0, MaterialRed);
		Mesh->SetMaterial(1, MaterialRed);
		Mesh->SetMaterial(2, MaterialRed);
	}
}

void AGuillotinePendulumTrap::SetPanel1MaterialBlue(bool Value)
{
	if (Value)
	{
		Panel1->SetMaterial(0, MaterialBlue);
		Panel1->SetMaterial(1, MaterialBlue);
	}
	else
	{
		Panel1->SetMaterial(0, MaterialRed);
		Panel1->SetMaterial(1, MaterialRed);
	}
}

void AGuillotinePendulumTrap::SetPanel2MaterialBlue(bool Value)
{
	if (Value)
	{
		Panel2->SetMaterial(0, MaterialBlue);
		Panel2->SetMaterial(1, MaterialBlue);
	}
	else
	{
		Panel2->SetMaterial(0, MaterialRed);
		Panel2->SetMaterial(1, MaterialRed);
	}
}

void AGuillotinePendulumTrap::ClearTriggerTimer()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(ClearTriggerTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(ClearTriggerTimerHandle);
	}

	GetWorld()->GetTimerManager().SetTimer(ClearTriggerTimerHandle, this, &AGuillotinePendulumTrap::ClearTriggerTimerCallback, 5, false);
}

void AGuillotinePendulumTrap::PlayMagneticSound()
{
	// 사운드
	if (MagneticSound)
	{
		// 특정 지점에서 사운드 재생
		UGameplayStatics::PlaySoundAtLocation(this, MagneticSound, GetActorLocation());
	}
}

void AGuillotinePendulumTrap::MoveSoundCallback()
{
	UGameplayStatics::PlaySoundAtLocation(this, CurrentMoveSound, GetActorLocation(), Volume);
}

void AGuillotinePendulumTrap::ReAttackTimeCallback()
{
	auto FirstPair = HitActorTimers.CreateConstIterator();
	OutHitActor(FirstPair->Key);
}

void AGuillotinePendulumTrap::ClearTriggerTimerCallback()
{
	//UE_LOG(LogTrap, Warning, TEXT("ClearTriggerTimer"));
	MeshMagneticTriggerActivatingPlayer = nullptr;
	Panel1MagneticTriggerActivatingPlayer = nullptr;
	Panel2MagneticTriggerActivatingPlayer = nullptr;
	MulticastRPCSetAllMaterialOriginal();
}

void AGuillotinePendulumTrap::DemolishGuillotine()
{
	if (!HasAuthority())
	{
		//UE_LOG(LogTrap, Warning, TEXT("Demolish"));
		SetActorEnableCollision(false);
	}

	NiagaraComponentSand->DeactivateImmediate();

	bIsActiveTrap = false;
	bCanAttack = true;
	bFirstRestrictionActivation = false;
	Mesh->SetWorldTransform(InitialMeshTransform);
	//Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	MeshMagneticTriggerActivatingPlayer = nullptr;
	Panel1MagneticTriggerActivatingPlayer = nullptr;
	Panel2MagneticTriggerActivatingPlayer = nullptr;

	OnHitActors.Reset();

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void AGuillotinePendulumTrap::SetMapmaker(bool IsAlive)
{
	if (IsAlive)
	{
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::GuillotineAlive);
	}
	else
	{
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::None);
	}
}

void AGuillotinePendulumTrap::MulticastRPCSpawnGC_Implementation()
{
	AActor* TriggerActor = GetOwner();
	ATrapSnapTrigger* TrapSnapTrigger = Cast<ATrapSnapTrigger>(TriggerActor);
	if (IsValid(TrapSnapTrigger))
	{
		TrapSnapTrigger->SetTrap(nullptr);
	}

	DemolishGuillotine();
	SetMapmaker(false);
}

void AGuillotinePendulumTrap::MulticastRPCSetTrapActive_Implementation(bool Value1, bool Value2)
{
	//UE_LOG(LogTrap, Warning, TEXT("MulticastRPCSetTrapActive_Implementation"));
	SetActiveTrap(Value1, Value2);
}

//void AGuillotinePendulumTrap::MulticastRPCActivateEffectt_Implementation(const FVector& ImpactPoint)
//{
//	UNiagaraSystem* NiagaraSystem = SandNiagara;
//
//	if (IsValid(NiagaraSystem))
//	{
//		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
//			GetWorld(),
//			NiagaraSystem,
//			ImpactPoint,
//			FRotator::ZeroRotator,
//			FVector(1.0f),
//			true,
//			true,
//			ENCPoolMethod::None,
//			true
//		);
//	}
//}

void AGuillotinePendulumTrap::MulticastRPCSetActivingPlayer_Implementation(AActor* OtherActor)
{
	AActor* OwnerActor = OtherActor->GetOwner();
	if (!OwnerActor)
	{
		//UE_LOG(LogTrap, Warning, TEXT("OwnerActor = nullptr"));
		return;
	}

	ActivatingPlayer = OwnerActor;
}

void AGuillotinePendulumTrap::MulticastRPCSpawnNiagaraSystem_Implementation(bool IsServer, const FVector& ImpactPoint)
{
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

void AGuillotinePendulumTrap::MulticastRPCSetMeshMaterialBlue_Implementation(bool Value)
{
	SetMeshMaterialBlue(Value);
}

void AGuillotinePendulumTrap::MulticastRPCSetPanel1MaterialBlue_Implementation(bool Value)
{
	SetPanel1MaterialBlue(Value);
}

void AGuillotinePendulumTrap::MulticastRPCSetPanel2MaterialBlue_Implementation(bool Value)
{
	SetPanel2MaterialBlue(Value);
}

void AGuillotinePendulumTrap::MulticastRPCSetMeshMaterialOriginal_Implementation()
{
	Mesh->SetMaterial(0, MeshOriginalMaterialElement0);
	Mesh->SetMaterial(1, MeshOriginalMaterialElement1);
	Mesh->SetMaterial(2, MeshOriginalMaterialElement2);
}

void AGuillotinePendulumTrap::MulticastRPCSetPanel1MaterialOriginal_Implementation(bool Value)
{
	if (Value)
	{
		Panel1->SetMaterial(0, PanelOriginalMaterialElement0);
		Panel1->SetMaterial(1, PanelOriginalMaterialElement1);
	}
	else
	{
		Panel2->SetMaterial(0, PanelOriginalMaterialElement0);
		Panel2->SetMaterial(1, PanelOriginalMaterialElement1);
	}
}


void AGuillotinePendulumTrap::MulticastRPCSetAllMaterialOriginal_Implementation()
{
	Mesh->SetMaterial(0, MeshOriginalMaterialElement0);
	Mesh->SetMaterial(1, MeshOriginalMaterialElement1);
	Mesh->SetMaterial(2, MeshOriginalMaterialElement2);
	Panel1->SetMaterial(0, PanelOriginalMaterialElement0);
	Panel1->SetMaterial(1, PanelOriginalMaterialElement1);
	Panel2->SetMaterial(0, PanelOriginalMaterialElement0);
	Panel2->SetMaterial(1, PanelOriginalMaterialElement1);
	Beam->SetMaterial(0, BeamOriginalMaterialElement1);
	Beam->SetMaterial(1, BeamOriginalMaterialElement1);
}

void AGuillotinePendulumTrap::MulticastRPCPlayHitSound_Implementation()
{
	if (CurrentHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CurrentHitSound, GetActorLocation());
	}
}

//void AGuillotinePendulumTrap::TickSwing()
//{
//	ActivateTrapTime += GetWorld()->GetDeltaSeconds();
//
//	// Sin(Time * 2 ) 의미 : 시간을 두 배 빠르게 흐르게 하겠다.
//	// -> sin 그래프 한 바퀴가 아마 3.14? 초인데 3.14 / 2 를 한 것 (아마도)
//	// 
//	// * 60 의미 : sin 그래프가 1 ~ -1  (y축? yaw? Value?) 를 의미하는데 60 ~ -60 을 하기 위함
//	float SinValue = FMath::Sin(ActivateTrapTime * test) * 70;
//
//	// 첫 번째 마그네틱 트리거 방향이 아니라면 반대로 스윙
//	if (!bFirstMagneticTriggerDirection)
//	{
//		SinValue *= -1;
//	}
//
//	//  time * ? 의미 : 그래프 value ? yaw? 값이 점점 줄어드는걸 원하는데 time을 그대로 넣었더니
//	// 너무 급하게 줄어들어서 0.x 값을 곱해서 천천히 줄어들게 만듬
//	//Yaw = Value / (Time *0.1);
//	// y = sin(x) * e^(-x*0.05)
//
//	float NewRoll = SinValue * FMath::Exp(-ActivateTrapTime * 0.05);
//
//	FRotator TrapRotator = GetActorRotation();
//	TrapRotator.Roll = NewRoll;
//	SetActorRotation(TrapRotator);
//
//	UE_LOG(LogTrap, Warning, TEXT("TrapRotator.Roll : %f"), TrapRotator.Roll);
//	UE_LOG(LogTrap, Warning, TEXT("ActivateTrapTime : %f"), ActivateTrapTime);
//}






//07 / 08 17:31   기요틴 진자.cpp 수정 전 원본 저장
//
//// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "Trap/GuillotinePendulumTrap.h"
//#include "Components/StaticMeshComponent.h"
//#include "Components/BoxComponent.h"
//#include "Components/SphereComponent.h"
//#include "Particles/ParticleSystemComponent.h"
//#include "Character/TrapperPlayer.h"
//#include "Kismet/GameplayStatics.h"
//#include "TrapperProject.h"
//#include "PhysicsEngine/PhysicsConstraintComponent.h"
//#include "Trap/TrabSnabEnviroment/GuillotineTrapSnabEnviroment.h"
//#include "Ping/MarkerComponent/MapMarkerComponent.h"
//#include "NiagaraComponent.h"
//#include "NiagaraSystem.h"
//#include "NiagaraFunctionLibrary.h"
//
//AGuillotinePendulumTrap::AGuillotinePendulumTrap()
//{
//	MapMarker->TrapType = EMapTrapType::GuillotineAlive;
//
//	PrimaryActorTick.bCanEverTick = true;
//
//	Beam = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Beam"));
//	Beam->SetupAttachment(DummyRoot);
//	Panel1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel1"));
//	Panel1->SetupAttachment(Beam);
//	Panel2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel2"));
//	Panel2->SetupAttachment(Beam);
//
//	Mesh->SetupAttachment(Beam);
//
//	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
//	TriggerSphere->SetupAttachment(Mesh);
//
//	ActivateTrapTime = 0.f;
//	ReEnableTime = 20.f;
//	ReAttackTime = 1.5f;
//	MaxRoll = 70.f;
//	bMagneticTriggerActivated = false;
//	bIsActiveTrap = false;
//	bFirstMagneticTriggerDirection = true;
//	bIsLerpingToTarget = false;
//	bLerpCompleted = false;
//
//	Damage = 150.f;
//}
//
//void AGuillotinePendulumTrap::BeginPlay()
//{
//	Super::BeginPlay();
//
//	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AGuillotinePendulumTrap::OnOverlapBeginArrow);
//
//	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AGuillotinePendulumTrap::OnOverapCharacter);
//
//	TrapPrevLocation = GetComponentByClass<UBoxComponent>()->GetComponentTransform().GetLocation();
//}
//
//void AGuillotinePendulumTrap::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//	//UE_LOG(LogTemp, Warning, TEXT("AGuillotinePendulumTrap Tick"));
//	if (bIsActiveTrap)
//	{
//		TickSwing();
//	}
//}
//
//void AGuillotinePendulumTrap::SetActiveTrap(bool Value/*활성화 여부*/, bool FirstMagneticTriggerDirection /*첫 번째 마그네틱 트리거 방향인지*/)
//{
//	//Mesh->SetCollisionProfileName(TEXT("NoCollision"));
//	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//
//	AGuillotineTrapSnabEnviroment* SnabEnviroment = Cast<AGuillotineTrapSnabEnviroment>(Owner);
//
//	//UE_LOG(LogTrap, Warning, TEXT("SetTrapActive"));
//	// 최종적으로 스냅 액터와 함정을 발동시킨 트리거 주체를 체크해서 
//	// 같은 사람이 아니라면 함정 발동 (1P, 2P 자성화살)
//	if (IsValid(Owner))
//	{
//
//		if (SnabEnviroment)
//		{
//			if (SnabEnviroment->GetActivatingPlayer() == ActivatingPlayer)
//			{
//				//UE_LOG(LogTrap, Warning, TEXT("SnabEnviroment->GetActivatingPlayer() == ActivatingPlayer"));
//				return;
//			}
//		}
//	}
//
//	// 이미 발동된 상태이므로 쿨타임 동안 재 발동 false
//	bCanAttack = false;
//
//	// 트랩 자성 화살 트리거 false
//	bMagneticTriggerActivated = false;
//
//	// 트랩 발동 누적 시간 초기화
//	ActivateTrapTime = 0.f;
//
//	// 스냅 환경 액터 자성 화살 트리거 false
//	SnabEnviroment->SetWallMagneticTriggerFalse();
//
//	// 트랩 활성화 (실질적 Tick 함수)
//	bIsActiveTrap = Value;
//
//	// 트랩 스윙 방향
//	bFirstMagneticTriggerDirection = FirstMagneticTriggerDirection;
//
//	// 진자 운동 시작 전 러프 
//	bIsLerpingToTarget = true;
//
//	// 함정 재발동 가능 타이머 설정
//	GetWorld()->GetTimerManager().SetTimer(ReEnableTimerHandle, this, &AGuillotinePendulumTrap::SetEnableCanAttack, ReEnableTime, false);
//}
//
//void AGuillotinePendulumTrap::SetEnableCanAttack()
//{
//	//UE_LOG(LogTrap, Warning, TEXT("SetEnableCanAttack"));
//	bCanAttack = true;
//}
//
//void AGuillotinePendulumTrap::TickSwing()
//{
//	/// Test
//	FVector CurrentLocation = GetComponentByClass<UBoxComponent>()->GetComponentTransform().GetLocation();
//	//TrapPrevLocation = GetComponentByClass<UBoxComponent>()->GetComponentTransform().GetLocation();
//
//	TrapFrameNormal = (CurrentLocation - TrapPrevLocation).GetSafeNormal();
//	LaunchSpeed = FVector::Distance(CurrentLocation, TrapPrevLocation);
//	//UE_LOG(LogTrap, Warning, TEXT("CurrentLocation : %s"), *CurrentLocation.ToString());
//	//UE_LOG(LogTrap, Warning, TEXT("TrapPrevLocation : %s"), *TrapPrevLocation.ToString());
//	//UE_LOG(LogTrap, Warning, TEXT("LaunchSpeed : %f"), LaunchSpeed);
//
//	ActivateTrapTime += GetWorld()->GetDeltaSeconds();
//
//	float NewRoll = 0.f;
//	float SinValue = 0.f;
//	float DampingFactor = 0.f;
//
//	if (ActivateTrapTime > 50.f)
//	{
//		static float LastLerp = 0.f;
//		LastLerp += GetWorld()->GetDeltaSeconds() * 0.2;
//		NewRoll = FMath::Lerp(GetActorRotation().Roll, 0.f, LastLerp);
//
//		if (LastLerp >= 1)
//		{
//			LastLerp = 0.f;
//			bIsActiveTrap = false;
//		}
//
//		FRotator TrapRotator = GetActorRotation();
//		TrapRotator.Roll = NewRoll;
//		SetActorRotation(TrapRotator);
//
//		return;
//	}
//
//	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 수치 보는 용도 (레전드 괴물) ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//
//	//CalculatePendulum(NewRoll, SinValue, DampingFactor);
//
//	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ 위에서 본 수치를 토대로 값 넣어주기 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//
//	// 타겟으로 러프
//	if (bIsLerpingToTarget)
//	{
//		float DestinationRoll = MaxRoll;
//
//		/// 조정 포인트 1
//		// 첫 번째 마그네틱 트리거 방향이 아니라면 반대로 스윙
//		if (!bFirstMagneticTriggerDirection)
//		{
//			DestinationRoll *= -1;
//			NewRoll = FMath::Lerp(GetActorRotation().Roll, DestinationRoll, GetWorld()->GetDeltaSeconds() * 2400 / FMath::Pow(FMath::Abs(GetActorRotation().Roll + 70), 1.5));
//		}
//		else
//		{
//			NewRoll = FMath::Lerp(GetActorRotation().Roll, DestinationRoll, GetWorld()->GetDeltaSeconds() * 2400 / FMath::Pow(FMath::Abs(GetActorRotation().Roll - 70), 1.5));
//		}
//
//		/// 조정 포인트 2
//		if (FMath::Abs(NewRoll) >= 67.3f)
//		{
//			NewRoll = (NewRoll > 0) ? 67.33f : -67.33f;
//			bIsLerpingToTarget = false;
//			bLerpCompleted = true;
//		}
//	}
//	// 러프가 끝나면 최초 한번 발동
//	else if (bLerpCompleted)
//	{
//		/// 조정 포인트 3 (위 수치 테스트에서 얻은 값 넣기)
//		ActivateTrapTime = 0.777386;
//		bLerpCompleted = false;
//
//		CalculatePendulum(NewRoll, SinValue, DampingFactor);
//
//		LastRoll = GetActorRotation().Roll;
//	}
//	// 본격적인 진자 운동 
//	else
//	{
//		CalculatePendulum(NewRoll, SinValue, DampingFactor);
//		CalculateDamage(NewRoll);
//		LastRoll = NewRoll;
//	}
//	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//	/// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//	///
//
//	FRotator TrapRotator = GetActorRotation();
//	TrapRotator.Roll = NewRoll;
//	SetActorRotation(TrapRotator);
//
//	TrapPrevLocation = CurrentLocation;
//
//	//UE_LOG(LogTrap, Warning, TEXT("TrapRotator.Roll : %f"), TrapRotator.Roll);
//	//UE_LOG(LogTrap, Warning, TEXT("ActivateTrapTime : %f"), ActivateTrapTime);
//	//UE_LOG(LogTrap, Warning, TEXT("Damage : %f"), Damage);
//}
//
//void AGuillotinePendulumTrap::AllMaterialBlue()
//{
//	//UE_LOG(LogTrap, Warning, TEXT("AllMaterialBlue"));
//	if (Mesh)
//	{
//		//UE_LOG(LogTrap, Warning, TEXT("AllMaterialBlue Mesh"));
//		Mesh->SetMaterial(0, MaterialBlue);
//		Mesh->SetMaterial(1, MaterialBlue);
//		Mesh->SetMaterial(2, MaterialBlue);
//	}
//	if (Beam)
//	{
//		Beam->SetMaterial(0, MaterialBlue);
//		Beam->SetMaterial(1, MaterialBlue);
//	}
//	if (Panel1)
//	{
//		Panel1->SetMaterial(0, MaterialBlue);
//		Panel1->SetMaterial(1, MaterialBlue);
//	}
//	if (Panel2)
//	{
//		Panel2->SetMaterial(0, MaterialBlue);
//		Panel2->SetMaterial(1, MaterialBlue);
//	}
//}
//
//void AGuillotinePendulumTrap::AllMaterialRed()
//{
//	if (Mesh)
//	{
//		Mesh->SetMaterial(0, MaterialRed);
//		Mesh->SetMaterial(1, MaterialRed);
//		Mesh->SetMaterial(2, MaterialRed);
//	}
//	if (Beam)
//	{
//		Beam->SetMaterial(0, MaterialRed);
//		Beam->SetMaterial(1, MaterialRed);
//	}
//	if (Panel1)
//	{
//		Panel1->SetMaterial(0, MaterialRed);
//		Panel1->SetMaterial(1, MaterialRed);
//	}
//	if (Panel2)
//	{
//		Panel2->SetMaterial(0, MaterialRed);
//		Panel2->SetMaterial(1, MaterialRed);
//	}
//}
//
//void AGuillotinePendulumTrap::OnOverlapBeginArrow(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
//{
//	if (!HasAuthority() || !bCanAttack)
//	{
//		return;
//	}
//
//	//UE_LOG(LogTrap, Warning, TEXT(" AGuillotinePendulumTrap::OnOverlapBegin"));
//
//	// 화살에 맞으면 땅에 떨어지게끔 물리 시뮬레이션 On
//	if (OtherActor && OtherActor->ActorHasTag("MagneticArrow"))
//	{
//		AActor* OwnerActor = OtherActor->GetOwner();
//		if (!OwnerActor)
//		{
//			return;
//		}
//
//		// 에임 피드백
//		AimFeedback(OtherActor);
//
//		// 자성화살 트리거 활성화
//		bMagneticTriggerActivated = true;
//
//		// 어떤 플레이어가 활성화 시켯는지 저장
//		//SetActivatingPlayer(OtherActor);
//		MulticastRPCSetActivingPlayer(OtherActor);
//
//		// 스냅 환경 액터 존재 유무 체크
//		if (IsValid(Owner))
//		{
//			AGuillotineTrapSnabEnviroment* SnabEnviroment = Cast<AGuillotineTrapSnabEnviroment>(Owner);
//			if (IsValid(SnabEnviroment))
//			{
//				// 환경 액터에 자성화살 트리거 체크 
//				if (SnabEnviroment->IsMagneticTriggerActivated())
//				{
//					// 트랩 활성화 -> Tick에서 함정 로테이션 변동 시작
//					//SetTrapActive(true, SnabEnviroment->IsFirstMagneticTriggerActivated(), SnabEnviroment);
//					MulticastRPCSetTrapActive(true, SnabEnviroment->IsFirstMagneticTriggerActivated());
//				}
//			}
//		}
//	}
//}
//
//void AGuillotinePendulumTrap::OnOverapCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
//{
//	if (!HasAuthority())
//	{
//		return;
//	}
//
//	if (bIsActiveTrap)
//	{
//		MulticastRPCActivateEffectt(SweepHitResult.ImpactPoint);
//	}
//
//	// 몬스터와 플레이어는 데미지를 받는다.
//	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()) && bIsActiveTrap)
//	{
//		// 이미 데미지를 받은 액터는 스킵
//		if (OtherActor && OnHitActors.Contains(OtherActor))
//		{
//			return;
//		}
//
//		UE_LOG(LogTrap, Warning, TEXT("OtherActor Name : %s"), *OtherActor->GetName());
//
//		// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//
//		FVector LaunchNormal = SweepHitResult.Normal;
//		FVector HitNormal;
//		FVector CalculateLocation;
//
//		if (LaunchNormal.Size() == 0)
//		{
//			CalculateLocation = OtherActor->GetActorLocation() - GetComponentByClass<UBoxComponent>()->GetComponentTransform().GetLocation();
//			LaunchNormal = CalculateLocation.GetSafeNormal();
//		}
//
//		HitNormal = LaunchNormal * -1;
//
//		float Power = TrapFrameNormal.Dot(HitNormal);
//		Power += 1;
//
//		FVector LaunchDirection;
//		FVector PlayerHitDirection;
//
//		if (Power > 0 && Power < 1)
//		{
//			//UE_LOG(LogTrap, Warning, TEXT("Power > 0 && Power < 1"));
//			PlayerHitDirection = LaunchNormal * Power * LaunchSpeed * 60;
//		}
//		else
//		{
//			//UE_LOG(LogTrap, Warning, TEXT("Power > 1"));
//			PlayerHitDirection = LaunchNormal * Power * LaunchSpeed * 90;
//		}
//
//		//넉백 처리
//		LaunchDirection = PlayerHitDirection * -1;
//
//		// 무조건 공중으론 날리자
//		if (LaunchDirection.Z < 0)
//		{
//			LaunchDirection.Z *= -1;
//		}
//
//		if (Damage > 30)
//		{
//			if (0 < LaunchDirection.X && LaunchDirection.X < 500)
//			{
//				LaunchDirection.X += 500;
//			}
//			else if (0 > LaunchDirection.X && LaunchDirection.X > -500)
//			{
//				LaunchDirection.X -= 500;
//			}
//
//			if (0 < LaunchDirection.Y && LaunchDirection.Y < 500)
//			{
//				LaunchDirection.Y += 500;
//			}
//			else if (0 > LaunchDirection.Y && LaunchDirection.Y > -500)
//			{
//				LaunchDirection.Y -= 500;
//			}
//
//			if (0 < LaunchDirection.Z && LaunchDirection.Z < 500)
//			{
//				LaunchDirection.Z += 500;
//			}
//			else if (0 > LaunchDirection.Z && LaunchDirection.Z < -500)
//			{
//				LaunchDirection.Z -= 500;
//			}
//
//			if (OtherActor->GetVelocity().Size() == 0)
//			{
//				LaunchDirection *= 2;
//			}
//		}
//		else
//		{
//			Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
//		}
//
//		if ((CalculateLocation.X > 0 && LaunchDirection.X < 0) || (CalculateLocation.X < 0 && LaunchDirection.X > 0))
//		{
//			/*UE_LOG(LogTrap, Warning, TEXT("-------------XXXXXXXXX-----------------------"));
//			UE_LOG(LogTrap, Warning, TEXT("CalculateLocation : %s"), *CalculateLocation.ToString());
//			UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());*/
//
//			LaunchDirection.X *= -1;
//
//			/*UE_LOG(LogTrap, Warning, TEXT("-------------------------------------"));
//			UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());
//			UE_LOG(LogTrap, Warning, TEXT("-------------------------------------"));*/
//		}
//		if ((CalculateLocation.Y > 0 && LaunchDirection.Y < 0) || CalculateLocation.Y < 0 && LaunchDirection.Y > 0)
//		{
//			/*UE_LOG(LogTrap, Warning, TEXT("-------------YYYYYYYYY-----------------------"));
//			UE_LOG(LogTrap, Warning, TEXT("CalculateLocation : %s"), *CalculateLocation.ToString());
//			UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());*/
//
//
//			LaunchDirection.Y *= -1;
//			/*UE_LOG(LogTrap, Warning, TEXT("-------------------------------------"));
//			UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());
//			UE_LOG(LogTrap, Warning, TEXT("-------------------------------------"));*/
//		}
//
//
//		Cast<ACharacter>(OtherActor)->LaunchCharacter(LaunchDirection, false, false);
//
//		/*UE_LOG(LogTrap, Warning, TEXT("HitNormal : %s"), *HitNormal.ToString());
//		UE_LOG(LogTrap, Warning, TEXT("Power : %f"), Power);
//		UE_LOG(LogTrap, Warning, TEXT("LaunchDirection : %s"), *LaunchDirection.ToString());
//		UE_LOG(LogTrap, Warning, TEXT("-------------------------------------"));*/
//
//		DrawDebugLine(GetWorld(), SweepHitResult.ImpactPoint, SweepHitResult.ImpactPoint + LaunchDirection, FColor::Red, true);
//
//		// ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
//
//		//UE_LOG(LogTrap, Warning, TEXT("Actor Name : %s"), *OtherActor->GetName());
//
//		// 액터에 데미지 전달
//		auto DamageTypeClass = UDamageType::StaticClass();
//		UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, this, DamageTypeClass);
//
//		// 타격을 입은 액터는 컨테이너에 추가
//		OnHitActors.Add(OtherActor);
//
//		// 타격을 입은 액터는 N초후 다시 데미지를 입을 수 있게 만듬.
//		// 타이머에 람다를 사용하여 인자와 함께 OutHitActor 호출
//		GetWorld()->GetTimerManager().SetTimer(
//			ReAttackTimerHandle,
//			[this, OtherActor]() { OutHitActor(OtherActor); },
//			ReAttackTime,
//			false
//		);
//
//		// 이펙트 활성화
//		//MulticastRPCActivateEffect();
//	}
//}
//
//void AGuillotinePendulumTrap::CalculatePendulum(float& NewRoll, float& SinValue, float& DampingFactor)
//{
//	// Sin(Time * 2 ) 의미 : 시간을 두 배 빠르게 흐르게 하겠다.
//	// -> sin 그래프 한 바퀴가 아마 3.14? 초인데 3.14 / 2 를 한 것 (아마도)
//	// 
//	// * 60 의미 : sin 그래프가 1 ~ -1  (y축? yaw? Value?) 를 의미하는데 60 ~ -60 을 하기 위함
//	// 
//	// time * ? 의미 : 그래프 value ? yaw? 값이 점점 줄어드는걸 원하는데 time을 그대로 넣었더니 너무 급하게 줄어들어서 0.x 값을 곱해서 천천히 줄어들게 만듬
//	// Yaw = Value / (Time *0.1);
//	// y = sin(x) * e^(-x*0.05)
//
//	SinValue = FMath::Sin(ActivateTrapTime * 2) * 70;
//	DampingFactor = FMath::Exp(-ActivateTrapTime * 0.05);
//
//	// 첫 번째 마그네틱 트리거 방향이 아니라면 반대로 스윙
//	if (!bFirstMagneticTriggerDirection)
//	{
//		SinValue *= -1;
//	}
//
//	NewRoll = SinValue * DampingFactor;
//}
//
//void AGuillotinePendulumTrap::CalculateDamage(const float& NewRoll)
//{
//	if (ActivateTrapTime > 10.f)
//	{
//		Damage = FMath::Abs((NewRoll - LastRoll) / GetWorld()->GetDeltaSeconds());
//
//		if (ActivateTrapTime > 40.f)
//		{
//			Damage = 10.f;
//		}
//		else if (ActivateTrapTime > 30.f)
//		{
//			Damage = 20.f;
//		}
//		else
//		{
//			if (Damage < 30.f)
//			{
//				Damage = 30.f;
//			}
//		}
//	}
//}
//
//void AGuillotinePendulumTrap::OutHitActor(AActor* OtherActor)
//{
//	if (OtherActor && OnHitActors.Contains(OtherActor))
//	{
//		UE_LOG(LogTrap, Warning, TEXT("OutHitActor"));
//		OnHitActors.Remove(OtherActor);
//	}
//}
//
//void AGuillotinePendulumTrap::SetActivatingPlayer(AActor* OtherActor)
//{
//	AActor* OwnerActor = OtherActor->GetOwner();
//	if (!OwnerActor)
//	{
//		return;
//	}
//	ActivatingPlayer = OwnerActor;
//}
//
//void AGuillotinePendulumTrap::MulticastRPCSetTrapActive_Implementation(bool Value1, bool Value2)
//{
//	//UE_LOG(LogTrap, Warning, TEXT("MulticastRPCSetTrapActive_Implementation"));
//	SetActiveTrap(Value1, Value2);
//}
//
//void AGuillotinePendulumTrap::MulticastRPCActivateEffectt_Implementation(const FVector& ImpactPoint)
//{
//	UNiagaraSystem* NiagaraSystem = SandNiagara;
//
//	if (IsValid(NiagaraSystem))
//	{
//		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
//			GetWorld(),
//			NiagaraSystem,
//			ImpactPoint,
//			FRotator::ZeroRotator,
//			FVector(1.0f),
//			true,
//			true,
//			ENCPoolMethod::None,
//			true
//		);
//	}
//}
//
//void AGuillotinePendulumTrap::MulticastRPCSetActivingPlayer_Implementation(AActor* OtherActor)
//{
//	AActor* OwnerActor = OtherActor->GetOwner();
//	if (!OwnerActor)
//	{
//		//UE_LOG(LogTrap, Warning, TEXT("OwnerActor = nullptr"));
//		return;
//	}
//	ActivatingPlayer = OwnerActor;
//}
//
////void AGuillotinePendulumTrap::TickSwing()
////{
////	ActivateTrapTime += GetWorld()->GetDeltaSeconds();
////
////	// Sin(Time * 2 ) 의미 : 시간을 두 배 빠르게 흐르게 하겠다.
////	// -> sin 그래프 한 바퀴가 아마 3.14? 초인데 3.14 / 2 를 한 것 (아마도)
////	// 
////	// * 60 의미 : sin 그래프가 1 ~ -1  (y축? yaw? Value?) 를 의미하는데 60 ~ -60 을 하기 위함
////	float SinValue = FMath::Sin(ActivateTrapTime * test) * 70;
////
////	// 첫 번째 마그네틱 트리거 방향이 아니라면 반대로 스윙
////	if (!bFirstMagneticTriggerDirection)
////	{
////		SinValue *= -1;
////	}
////
////	//  time * ? 의미 : 그래프 value ? yaw? 값이 점점 줄어드는걸 원하는데 time을 그대로 넣었더니
////	// 너무 급하게 줄어들어서 0.x 값을 곱해서 천천히 줄어들게 만듬
////	//Yaw = Value / (Time *0.1);
////	// y = sin(x) * e^(-x*0.05)
////
////	float NewRoll = SinValue * FMath::Exp(-ActivateTrapTime * 0.05);
////
////	FRotator TrapRotator = GetActorRotation();
////	TrapRotator.Roll = NewRoll;
////	SetActorRotation(TrapRotator);
////
////	UE_LOG(LogTrap, Warning, TEXT("TrapRotator.Roll : %f"), TrapRotator.Roll);
////	UE_LOG(LogTrap, Warning, TEXT("ActivateTrapTime : %f"), ActivateTrapTime);
////}