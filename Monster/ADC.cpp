// Fill out your copyright notice in the Description page of Project Settings.


#include "ADC.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "Framework/TrapperGameState.h"
#include "Monster/Weapon/Wand.h"
#include "Monster/Weapon/MagicBall.h"
#include "Data/MonsterDataTables.h"
#include "Monster/DeadbodyParts.h"
#include "Turret/Turret.h"
#include "Tower/Tower.h"

AADC::AADC()
{
	PrimaryActorTick.bCanEverTick = false;

	DeadbodyHornSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HornSpawnPoint"));
	DeadbodyHornSpawnPoint->SetupAttachment(RootComponent);

	DeadbodyHeadSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HeadSpawn"));
	DeadbodyHeadSpawnPoint->SetupAttachment(RootComponent);	

	DeadbodyChestSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ChestSpawnPoint"));
	DeadbodyChestSpawnPoint->SetupAttachment(RootComponent);

	DeadbodyLeftHandSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHandSpawnPoint"));
	DeadbodyLeftHandSpawnPoint->SetupAttachment(RootComponent);

	DeadbodyRightHandSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RightHandSpawnPoint"));
	DeadbodyRightHandSpawnPoint->SetupAttachment(RootComponent);

	DeadbodyLegSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LegSpawnPoint"));
	DeadbodyLegSpawnPoint->SetupAttachment(RootComponent);

	DeadbodyRightLegSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RightLegSpawnPoint"));
	DeadbodyRightLegSpawnPoint->SetupAttachment(RootComponent);

	DeadbodyFootSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("FootSpawnPoint"));
	DeadbodyFootSpawnPoint->SetupAttachment(RootComponent);

}

void AADC::BeginPlay()
{
	Super::BeginPlay();

	Wand = GetWorld()->SpawnActor<AWand>(WandClass);
	Wand->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("Wand"));
	Wand->SetOwner(this);

	///SetData
	//HP
	if (MonsterInfoTable[1].HP)
	{
		MaxHP = MonsterInfoTable[1].HP;
		CurrentHP = MaxHP;
	}
	//Damage
	if (MonsterInfoTable[1].Attack)
	{
		AttackDamage = MonsterInfoTable[1].Attack;
	}
	//Speed
	if (MonsterInfoTable[1].MoveSpeed)
	{
		MonsterSpeed = MonsterInfoTable[1].MoveSpeed;
		GetCharacterMovement()->MaxWalkSpeed = MonsterSpeed;
	}
	if (MonsterInfoTable[1].ChaseSpeed)
	{
		MonsterDetectSpeed = MonsterInfoTable[1].ChaseSpeed;
	}
	if (MonsterInfoTable[1].ChaseSpeed)
	{
		MonsterEffectSpeed = MonsterInfoTable[1].EffectSpeed;
	}
	//DetectTime
	if (MonsterInfoTable[1].LocationTimer)
	{
		DetectTime = MonsterInfoTable[1].LocationTimer;
	}
	//Item
	if (MonsterInfoTable[1].SpawnAmount)
	{
		ItemMinDropNumber = MonsterInfoTable[1].SpawnAmount - 1;
		ItemMaxDropNumber = MonsterInfoTable[1].SpawnAmount;
	}
	//Debuffe Timer
	if (MonsterInfoTable[1].SpawnAmount)
	{
		DebuffeTime = MonsterInfoTable[1].EffectTimer * 10;
	}
}

void AADC::SpawnMagicBall()
{
	//Wand 불러오기
	if (IsValid(Wand))
	{
		FVector SpawnLocation = Wand->MagicBallSpawn->GetComponentLocation();
		FRotator SpawnRotation = Wand->MagicBallSpawn->GetComponentRotation();

		auto ADCAttack = GetWorld()->SpawnActor<AMagicBall>(Wand->MagicBall, SpawnLocation, SpawnRotation);
		ADCAttack->TargetCharacter = Cast<ACharacter>(DetectPlayer);
		ADCAttack->Tower = Cast<ATower>(Tower);
		ADCAttack->TargetTurret = Cast<ATurret>(DetectTurret);
		
		ADCAttack->bIsPlayerDetected = this->bIsPlayerDetected;
		ADCAttack->bIsTurretDetected = this->bIsTurretDetected;
		ADCAttack->bIsTowerDetected  = this->bIsTowerDetected;

		if (MonsterInfoTable[1].Attack)
		{
			ADCAttack->Damage = AttackDamage;
		}
		ADCAttack->SetOwner(this);

		//UE_LOG(LogTemp, Warning, TEXT("Fire!"));
	}
}

void AADC::AttackADCMontage()
{
	ServerRPCAttack();
}

void AADC::ChangeADCDamage()
{
	float DamageVairiation = TrapperGameState->MonsterAttackDamage;

	AttackDamage = AttackDamage - (AttackDamage * DamageVairiation);
}

void AADC::ADCDeadByTrap()
{
	if (IsValid(DeadbodyHorn) && IsValid(DeadbodyHead) && IsValid(DeadbodyChest)
		&& IsValid(DeadbodyLeftHand) && IsValid(DeadbodyRightHand) && IsValid(DeadbodyLeg)
		&& IsValid(DeadbodyRightLeg) && IsValid(DeadbodyFoot))
	{
		FVector HornSpawnLocation = DeadbodyHornSpawnPoint->GetComponentLocation();
		FRotator HornSpawnRotation = DeadbodyHornSpawnPoint->GetComponentRotation();

		FVector HeadSpawnLocation = DeadbodyHeadSpawnPoint->GetComponentLocation();
		FRotator HeadSpawnRotation = DeadbodyHeadSpawnPoint->GetComponentRotation();

		FVector ChestSpawnLocation = DeadbodyChestSpawnPoint->GetComponentLocation();
		FRotator ChestSpawnRotation = DeadbodyChestSpawnPoint->GetComponentRotation();

		FVector LeftHandSpawnLocation = DeadbodyLeftHandSpawnPoint->GetComponentLocation();
		FRotator LeftHandSpawnRotation = DeadbodyLeftHandSpawnPoint->GetComponentRotation();

		FVector RightHandSpawnLocation = DeadbodyRightHandSpawnPoint->GetComponentLocation();
		FRotator RightHandSpawnRotation = DeadbodyRightHandSpawnPoint->GetComponentRotation();

		FVector LegSpawnLocation = DeadbodyLegSpawnPoint->GetComponentLocation();
		FRotator LegSpawnRotation = DeadbodyLegSpawnPoint->GetComponentRotation();

		FVector RightLegSpawnLocation = DeadbodyRightLegSpawnPoint->GetComponentLocation();
		FRotator RightLegSpawnRotation = DeadbodyRightLegSpawnPoint->GetComponentRotation();

		FVector FootSpawnLocation = DeadbodyFootSpawnPoint->GetComponentLocation();
		FRotator FootSpawnRotation = DeadbodyFootSpawnPoint->GetComponentRotation();

		TObjectPtr<ADeadbodyParts> HornPart = GetWorld()->SpawnActor<ADeadbodyParts>(DeadbodyHorn, HornSpawnLocation, HornSpawnRotation);
		TObjectPtr<ADeadbodyParts> HeadPart = GetWorld()->SpawnActor<ADeadbodyParts>(DeadbodyHead, HeadSpawnLocation, HeadSpawnRotation);
		TObjectPtr<ADeadbodyParts> ChestPart = GetWorld()->SpawnActor<ADeadbodyParts>(DeadbodyChest, ChestSpawnLocation, ChestSpawnRotation);
		TObjectPtr<ADeadbodyParts> LeftHand = GetWorld()->SpawnActor<ADeadbodyParts>(DeadbodyLeftHand, LeftHandSpawnLocation, LeftHandSpawnRotation);
		TObjectPtr<ADeadbodyParts> RightHand = GetWorld()->SpawnActor<ADeadbodyParts>(DeadbodyRightHand, LeftHandSpawnLocation, RightHandSpawnRotation);
		TObjectPtr<ADeadbodyParts> Leg = GetWorld()->SpawnActor<ADeadbodyParts>(DeadbodyLeg, LegSpawnLocation, LegSpawnRotation);
		TObjectPtr<ADeadbodyParts> RightLeg = GetWorld()->SpawnActor<ADeadbodyParts>(DeadbodyRightLeg, RightLegSpawnLocation, RightLegSpawnRotation);
		TObjectPtr<ADeadbodyParts> Foot = GetWorld()->SpawnActor<ADeadbodyParts>(DeadbodyFoot, FootSpawnLocation, FootSpawnRotation);

	}
}

void AADC::ServerRPCAttack_Implementation()
{
	MulticastRPCAttack();
}

void AADC::MulticastRPCAttack_Implementation()
{
	float RandomNum = FMath::FRandRange(1.0, 3.0);

	if (RandomNum < 2)
	{
		AnimInstance->Montage_Play(AttackkActionMontage1);
	}

	else
	{
		AnimInstance->Montage_Play(AttackkActionMontage2);
	}
}

