// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Wand.generated.h"

UCLASS()
class TRAPPERPROJECT_API AWand : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWand();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> WandMesh;

	//원딜몬스터의 공격 방식
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attact")
	TObjectPtr<class USceneComponent> MagicBallSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	TSubclassOf<class AMagicBall> MagicBall;

};
