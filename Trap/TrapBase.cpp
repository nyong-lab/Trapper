// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/TrapBase.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Character/TrapperPlayer.h"
#include "Monster/ProjectileMonster.h"
#include "Net/UnrealNetwork.h"
#include "Bow/Arrow.h"
#include "Ping/MarkerComponent/MapMarkerComponent.h"
#include "TrapperProject.h"
#include "Trap/Component/TrapMechanics.h"
#include "Framework/TrapperGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/TrapperPlayerController.h"
#include "Widget/MultikillHUD.h"


ATrapBase::ATrapBase()
{
	// 리플리케이트
	bReplicates = true;

	// 틱 사용 안함
	//PrimaryActorTick.bCanEverTick = false;

	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));

	RootComponent = DummyRoot;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	//DemolishTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("DemolishTrigger"));
	EffectSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Effect"));
	EffectParticle = CreateDefaultSubobject<UParticleSystem>(TEXT("EffectParticle"));

	Mesh->SetupAttachment(DummyRoot);
	Trigger->SetupAttachment(Mesh);
	//DemolishTrigger->SetupAttachment(Mesh);
	EffectSystem->SetupAttachment(Mesh);

	bCanAttack = true;
	bWreckage = false;
	InstallCost = 0;
	TrapType = ETrapType::Spike;
	bVolatile = false;
	bTutorialTrap = false;

	MapMarkerwpqkfwpqkf = CreateDefaultSubobject<UMapMarkerComponent>(TEXT("MapMarkerComponent"));
	//MapMarker->ObjectType = EMapObjectType::Trap;
	//MapMarker->TrapType = EMapTrapType::SpikeAlive;
	AddOwnedComponent(MapMarkerwpqkfwpqkf);

	EffectSystem->bAutoActivate = false;

	//WreckageMaterial;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> WreckageFinder(TEXT("/Script/Engine.Material'/Game/Trap/Material/M_Wreckage.M_Wreckage'"));
	if (WreckageFinder.Succeeded())
	{
		WreckageMaterial = WreckageFinder.Object;
	}

	Tags.Add(FName("Trap"));
}

void ATrapBase::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTrap, Warning, TEXT("ATrapBase::BeginPlay"));
	//UE_LOG(LogTrap, Warning, TEXT("GetName : %s"), *GetName());

	if (HasAuthority())
	{
		TrapperGameState = GetWorld()->GetGameState<ATrapperGameState>();
		if (IsValid(TrapperGameState.Get()))
		{
			//UE_LOG(LogTrap, Warning, TEXT("GameState true"));
			Damage = Damage * TrapperGameState->TrapDamage;
		}
	}
}

void ATrapBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATrapBase, TrapType);
	DOREPLIFETIME(ATrapBase, bVolatile);
	DOREPLIFETIME(ATrapBase, InstallCost);
	DOREPLIFETIME(ATrapBase, Speed);
	DOREPLIFETIME(ATrapBase, IncreaseSpeed);
}

void ATrapBase::SetEffectParticle(class UParticleSystem* Value)
{
	EffectSystem->SetTemplate(Value);
}

void ATrapBase::AimFeedback(AActor* OtherActor)
{
	ATrapperPlayer* player = Cast<ATrapperPlayer>(OtherActor->GetOwner());
	AArrow* arrow = Cast<AArrow>(OtherActor);

	if (player && arrow)
	{
		player->ShowHitFeedbackHUD(false, arrow->IsFullCharged());
	}
}

void ATrapBase::UpgradeDamage(float DamageIncreaseDecreaseRate)
{
	//UE_LOG(LogTrap, Warning, TEXT("Damage : %f"), Damage);

	DamageRate = DamageIncreaseDecreaseRate;

	//UE_LOG(LogTrap, Warning, TEXT("DamageIncreaseDecreaseRate : %f"), DamageIncreaseDecreaseRate);
	//UE_LOG(LogTrap, Warning, TEXT("UgradeDamage : %f"), Damage);
}

void ATrapBase::SetDataTableInfo(float DataInstallCost, float DataDamage, float DataIncreaseDamage, float DataSpeed, float DataIncreaseSpeed, float DataStrength, float DataIncreaseStrength, float DataDuration)
{
	InstallCost = DataInstallCost;
	Damage = DataDamage;
	IncreaseDamage = DataIncreaseDamage;
	Speed = DataSpeed;
	IncreaseSpeed = DataIncreaseSpeed;
	Strength = DataStrength;
	IncreaseStrength = DataIncreaseStrength;
	Duration = DataDuration;
}

void ATrapBase::CheckKillCount(AController* EventInstigator, AActor* DamageCauser)
{
	if (!EventInstigator || !DamageCauser)
	{
		return;
	}

	if (IsValid(TrapperGameState.Get()))
	{
		TrapperGameState->CheckKillCount(EventInstigator, DamageCauser);
	}
}

void ATrapBase::ResetKillCount(AActor* DamageCauser)
{
	if (IsValid(TrapperGameState.Get()))
	{
		TrapperGameState->ResetKillCount(DamageCauser);
	}
}

void ATrapBase::CheckAndResetKillCount(AController* EventInstigator, AActor* DamageCauser)
{
	if (!EventInstigator || !DamageCauser)
	{
		return;
	}

	if (IsValid(TrapperGameState.Get()))
	{
		TrapperGameState->CheckAndResetKillCount(EventInstigator, DamageCauser);
	}
}

void ATrapBase::CheckKillCountCollaboration(AActor* DamageCauser)
{
	if (!DamageCauser)
	{
		return;
	}

	if (IsValid(TrapperGameState.Get()))
	{
		TrapperGameState->CheckKillCountCollaboration(DamageCauser);
	}
}

void ATrapBase::CheckAndResetKillCountCollaboration(AActor* DamageCauser)
{
	if (!DamageCauser)
	{
		return;
	}

	if (IsValid(TrapperGameState.Get()))
	{
		TrapperGameState->CheckAndResetKillCountCollaboration(DamageCauser);
	}
}

//void ATrapBase::OnOverlapBeginDemolishTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
//{
//	/*if (!HasAuthority())
//	{
//		return;
//	}*/
//
//	if (!OtherActor->ActorHasTag("Player"))
//	{
//		return;
//	}
//
//	ATrapperPlayer* Player = Cast<ATrapperPlayer>(OtherActor);
//	if (!IsValid(Player) || !Player->IsLocallyControlled())
//	{
//		return;
//	}
//
//	UTrapMechanics* TrapMechanics = Player->TrapMechanics;
//	if (!IsValid(TrapMechanics))
//	{
//		return;
//	}
//
//	TrapMechanics->StartDemolishRaycast(this);
//}

//void ATrapBase::OnOverlapEndDemolishTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//	if (!HasAuthority())
//	{
//		return;
//	}
//
//	if (!OtherActor->ActorHasTag("Player"))
//	{
//		return;
//	}
//
//	ATrapperPlayer* Player = Cast<ATrapperPlayer>(OtherActor);
//	if (!IsValid(Player))
//	{
//		return;
//	}
//
//	UTrapMechanics* TrapMechanics = Player->TrapMechanics;
//	if (!IsValid(TrapMechanics))
//	{
//		return;
//	}
//
//	TrapMechanics->EndDemolishRaycast(this);
//}

void ATrapBase::ChangeWreckageMaterial(UStaticMeshComponent* WreckageMesh)
{
	if (!IsValid(WreckageMesh))
	{
		return;
	}

	int32 Num = WreckageMesh->GetNumMaterials();

	//UE_LOG(LogTrap, Warning, TEXT("Num : %d"), Num);
	//UE_LOG(LogTrap, Warning, TEXT("WreckageMesh Name : %s"), *WreckageMesh->GetName());

	for (int i = 0; i < Num; ++i)
	{
		WreckageMesh->SetMaterial(i, WreckageMaterial);
	}
}

void ATrapBase::ChangeAliveMapMarker()
{
	switch (TrapType)
	{
	case ETrapType::Bear:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::BearAlive);
		break;
	case ETrapType::GunpowderBarrel:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::BarrelAlive);
		break;
	case ETrapType::Plank:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::PlankAlive);
		break;
	case ETrapType::OilBag:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::OilBagAlive);
		break;
	case ETrapType::Clap:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::ClapAlive);
		break;
	case ETrapType::Spear:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::SpearAlive);
		break;
	}
}

void ATrapBase::ChangeWreckageMapMarker()
{
	switch (TrapType)
	{
	case ETrapType::Bear:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::BearWreckage);
		break;
	case ETrapType::GunpowderBarrel:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::BarrelWreckage);
		break;
	case ETrapType::Plank:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::PlankWreckage);
		break;
	case ETrapType::OilBag:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::OilBagWreckage);
		break;
	case ETrapType::Clap:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::ClapWreckage);
		break;
	case ETrapType::Spear:
		MapMarkerwpqkfwpqkf->SetTrapType(EMapTrapType::SpearWreckage);
		break;
	}
}

void ATrapBase::OnTrapActivatedBroadcast()
{
	//UE_LOG(LogTrap, Warning, TEXT("OnTrapActivated.Broadcast();"));
	OnTrapActivated.Broadcast();
}

