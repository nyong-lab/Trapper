// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/Range/ClapRange.h"

// Sets default values
AClapRange::AClapRange()
{
	// 리플리케이트
	bReplicates = true;

	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));
	RootComponent = DummyRoot;

	BlueRangeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlueRangeMesh"));
	RedRangeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RedRangeMesh"));

	BlueRangeMesh->SetupAttachment(DummyRoot);
	RedRangeMesh->SetupAttachment(DummyRoot);
}

// Called when the game starts or when spawned
void AClapRange::BeginPlay()
{
	Super::BeginPlay();
	
}

