// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Weapon/Wand.h"

// Sets default values
AWand::AWand()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WandMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wand"));
	RootComponent = WandMesh;
	
	MagicBallSpawn = CreateDefaultSubobject<USceneComponent>(TEXT("MagicBall Spawn"));
	MagicBallSpawn->SetupAttachment(WandMesh);
}

// Called when the game starts or when spawned
void AWand::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void AWand::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
}

