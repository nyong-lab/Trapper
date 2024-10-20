// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Oil.generated.h"


UCLASS()
class TRAPPERPROJECT_API AOil : public AActor
{
	GENERATED_BODY()

public:
	AOil();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void SetReduceTime(float Duration);

protected:
	UFUNCTION()
	void OnOverlapBeginCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	UFUNCTION()
	void OnOverlapEndCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void ReduceCharacterSpeed(ACharacter* Character);
	void RestoreCharacterSpeed(ACharacter* Character);
	void ReducedSpeedTimerCallback();

private:
	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UBoxComponent> Trigger;

	// 이동속도 감소 퍼센트
	UPROPERTY(EditAnywhere, Category = "Trap")
	float SlowReductionRate;

	// 이동속도 감소 시간
	UPROPERTY(EditAnywhere, Category = "Trap")
	float ReducedSpeedTime;

	TSet<TObjectPtr<AActor>> OverlappingActors;

	FTimerHandle ReducedSpeedTimerHandle;

	/// RPC --------------------------------------------------------
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCWreckActivation();
};


