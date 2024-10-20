// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClapRange.generated.h"

UCLASS()
class TRAPPERPROJECT_API AClapRange : public AActor
{
	GENERATED_BODY()
	
public:	
	AClapRange();

protected:
	virtual void BeginPlay() override;

	/*UFUNCTION()
	void OnOverlapBeginClap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);*/

private:
	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class USceneComponent> DummyRoot;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> BlueRangeMesh;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> RedRangeMesh;

};
