// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/TrabSnabEnviroment/TrapSnabEnviromentBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "TrapperProject.h"
#include "Trap/TrapBase.h"


// Sets default values
ATrapSnabEnviromentBase::ATrapSnabEnviromentBase()
{
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SnapPoint = CreateDefaultSubobject<UBoxComponent>(TEXT("SnapPoint"));

	SnapTriggerrrrrrrrrrr = CreateDefaultSubobject<UBoxComponent>(TEXT("SnapTrigger"));
	CheckInstallTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("CheckInstallTrigger"));
	
	RootComponent = DummyRoot;
	Mesh->SetupAttachment(DummyRoot);
	SnapPoint->SetupAttachment(Mesh);
	CheckInstallTrigger->SetupAttachment(Mesh);

	SnapTriggerrrrrrrrrrr->SetupAttachment(Mesh);

	SnapPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bInstalledTrap = false;
}

void ATrapSnabEnviromentBase::BeginPlay()
{
	Super::BeginPlay();

	SnapTriggerBoxes.Add(SnapTriggerrrrrrrrrrr);

	CheckInstallTrigger->OnComponentBeginOverlap.AddDynamic(this, &ATrapSnabEnviromentBase::OnOverlapBegin);
	CheckInstallTrigger->OnComponentEndOverlap.AddDynamic(this, &ATrapSnabEnviromentBase::OnOverlapEnd);
}

void ATrapSnabEnviromentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATrapSnabEnviromentBase, bInstalledTrap);
	DOREPLIFETIME(ATrapSnabEnviromentBase, InstalledTrap);
}

void ATrapSnabEnviromentBase::SetInstalledTrap(bool Value)
{
	bInstalledTrap = Value;

	/*if (!bInstalledTrap)
	{
		InstalledTrap = nullptr;
	}*/
}
const ATrapBase* ATrapSnabEnviromentBase::FindChildTrap() const
{
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	for (const AActor* AttachedActor : AttachedActors)
	{
		if (const ATrapBase* Trap = Cast<ATrapBase>(AttachedActor))
		{
			return Trap;
		}
	}

	return nullptr;
}

void ATrapSnabEnviromentBase::SetTrap(class ATrapBase* Trap)
{
	InstalledTrap = Trap;
	InstalledTrap->SetOwner(this);
	bInstalledTrap = true;
}

bool ATrapSnabEnviromentBase::IsCanInstall() const
{
	if (OverlappingActors.Num() == 0)
	{
		return true;
	}

	//UE_LOG(LogTrap, Warning, TEXT("OverlappingActors.Num() : %d"), OverlappingActors.Num());

	return false;
}

void ATrapSnabEnviromentBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	if (OtherActor)
	{
		OverlappingActors.Add(OtherActor);
	}
}

void ATrapSnabEnviromentBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		OverlappingActors.Remove(OtherActor);
	}
}

void ATrapSnabEnviromentBase::OnRep_InstalledTrap()
{
	// GuillotinePendulumTrap 함정에서 진자운동을 클라에서 돌려야하기때문에
	// Owner 관계를 맺어줌  2024.06.21
	InstalledTrap->SetOwner(this);
	bInstalledTrap = true;
}


