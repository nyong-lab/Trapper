// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Trap/GuillotinePendulumTrap.h"
#include "TrapSnapTrigger.generated.h"

UCLASS()
class TRAPPERPROJECT_API ATrapSnapTrigger : public AActor
{
	GENERATED_BODY()

public:
	ATrapSnapTrigger();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	TObjectPtr<class UBoxComponent> GetSnapPoint() const { return SnapPoint; }
	void SetInstalledTrap(bool Value);
	void SetTrap(class ATrapBase* Trap);
	FORCEINLINE UBoxComponent* GetSnabTriggerBox() const { return SnapTrigger; }
	FORCEINLINE bool IsInstalledTrap() const { return bInstalledTrap; }
	FORCEINLINE class ATrapBase* GetSnapGuillotine() const { return SnapGuillotine; }
	void SpawnSnapGuillotine();

private:
	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class USceneComponent> DummyRoot;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UBoxComponent> SnapPoint;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UBoxComponent> SnapTrigger;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TSubclassOf<class AGuillotinePendulumTrap> GuillotinePendulumTrapSubClass;

	UPROPERTY(ReplicatedUsing = OnRep_SnapGuillotine)
	TObjectPtr<class ATrapBase> SnapGuillotine;


	/// 리플리케이션 -----------------------------------------------
	UPROPERTY(ReplicatedUsing = OnRep_InstalledTrap)
	TObjectPtr<class ATrapBase> InstalledTrap;

	UPROPERTY(Replicated)
	uint8 bInstalledTrap : 1;

	/// OnRep -----------------------------------------------------
	UFUNCTION()
	void OnRep_InstalledTrap();

	UFUNCTION()
	void OnRep_SnapGuillotine();
};
