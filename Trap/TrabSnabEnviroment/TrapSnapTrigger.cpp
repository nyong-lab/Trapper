// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/TrabSnabEnviroment/TrapSnapTrigger.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "TrapperProject.h"
#include "Trap/TrapBase.h"

ATrapSnapTrigger::ATrapSnapTrigger()
{
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));
	SnapPoint = CreateDefaultSubobject<UBoxComponent>(TEXT("SnapPoint"));
	SnapTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("SnapTrigger"));

	RootComponent = DummyRoot;
	SnapPoint->SetupAttachment(DummyRoot);
	SnapTrigger->SetupAttachment(DummyRoot);

	SnapPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bInstalledTrap = false;
}

void ATrapSnapTrigger::BeginPlay()
{
	Super::BeginPlay();

	/*if (HasAuthority())
	{
		SnapGuillotine = GetWorld()->SpawnActor<ATrapBase>(GuillotinePendulumTrapSubClass, SnapPoint->GetComponentTransform());
		if (SnapGuillotine)
		{
			SnapGuillotine->SetActorHiddenInGame(true);
			SnapGuillotine->SetActorEnableCollision(false);
			SnapGuillotine->ForceNetRelevant();
			SnapGuillotine->ForceNetUpdate();
		}
	}*/
}

void ATrapSnapTrigger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATrapSnapTrigger, bInstalledTrap);
	DOREPLIFETIME(ATrapSnapTrigger, InstalledTrap);
	DOREPLIFETIME(ATrapSnapTrigger, SnapGuillotine);
}

void ATrapSnapTrigger::SetInstalledTrap(bool Value)
{
	bInstalledTrap = Value;
}

void ATrapSnapTrigger::SetTrap(class ATrapBase* Trap)
{
	//UE_LOG(LogTrap, Warning, TEXT("	ATrapSnapTrigger::SetTrap"));
	InstalledTrap = Trap;

	if(IsValid(InstalledTrap))
	{
		InstalledTrap->SetOwner(this);
		bInstalledTrap = true;
	}
	else
	{
		bInstalledTrap = false;
	}
}

void ATrapSnapTrigger::SpawnSnapGuillotine()
{
	//UE_LOG(LogTrap, Warning, TEXT("SpawnSnapGuillotine"));

	/*ForceNetRelevant();
	ForceNetUpdate();*/

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SpawnSnapGuillotine"));
	SnapGuillotine = GetWorld()->SpawnActor<ATrapBase>(GuillotinePendulumTrapSubClass, SnapPoint->GetComponentTransform());
	if (SnapGuillotine)
	{
		SnapGuillotine->SetOwner(this);
		SnapGuillotine->SetActorHiddenInGame(true);
		SnapGuillotine->SetActorEnableCollision(false);
		/*SnapGuillotine->ForceNetRelevant();
		SnapGuillotine->ForceNetUpdate();*/
	}
}

void ATrapSnapTrigger::OnRep_InstalledTrap()
{
	//UE_LOG(LogTrap, Warning, TEXT("OnRep_InstalledTrap"));

	// GuillotinePendulumTrap 함정에서 진자운동을 클라에서 돌려야하기때문에
	// Owner 관계를 맺어줌  2024.06.21
	if (InstalledTrap)
	{
		InstalledTrap->SetOwner(this);
		bInstalledTrap = true;
	}
}

void ATrapSnapTrigger::OnRep_SnapGuillotine()
{
	//UE_LOG(LogTrap, Warning, TEXT("OnRep_SnapGuillotine"));
	if (SnapGuillotine)
	{
		SnapGuillotine->SetActorHiddenInGame(true);
		SnapGuillotine->SetActorEnableCollision(ECollisionEnabled::NoCollision);
	}
}
