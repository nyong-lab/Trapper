// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BossPhaseCheckBox.h"
#include "Components/BoxComponent.h"

#include "Monster/Boss/Boss_Phase1.h"
#include "Monster/Boss/Boss_Phase2.h"

// Sets default values
ABossPhaseCheckBox::ABossPhaseCheckBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);

	//Add Dynamic
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABossPhaseCheckBox::OnOverlapBeginTriggerBox);
}

// Called when the game starts or when spawned
void ABossPhaseCheckBox::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABossPhaseCheckBox::OnOverlapBeginTriggerBox(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag("BossPhase1"))
	{
		TObjectPtr<ABoss_Phase1> Boss = Cast<ABoss_Phase1>(OtherActor);
		Boss->bIsPhaseChange = true;

		UE_LOG(LogTemp, Warning, TEXT("Boss Phase Change"));
	}

}


