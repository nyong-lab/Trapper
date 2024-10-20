// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Trap/TrapBase.h"
#include "OilBagTrap.generated.h"

/// <summary>
/// 기름 주머니 함정
/// </summary>
UCLASS()
class TRAPPERPROJECT_API AOilBagTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	AOilBagTrap();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void WreckageActivation();
	virtual void ReInstall() override;
	virtual void Demolish() override;

protected:
	UFUNCTION()
	void OnHitMeshToArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnHitRopeToArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnNiagaraSystemFinished(UNiagaraComponent* FinishedComponent);

private:
	void ActivateTrapExplosion();
	void ActivateTrapSimulation();
	void CheckOilVelocity();
	void ChangeOriginalMaterial();
	void PlaySound();
	void HiddenCallback();
	void FallingOilCallback();
	void SpreadingOilCallback();

	UPROPERTY(EditAnywhere, Category = "GC")
	TSubclassOf<class AActor> OilGC;

	/// RPC --------------------------------------------------------
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCActivateEffect();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSetCollisionResponseOfSimulating();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSetHiddenMeshOfVelocity();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSpreadingOil(FVector Location);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> ActivateSound;

	UPROPERTY(EditDefaultsOnly, Category = "Trap")
	USceneComponent* FallingOilNiagaraAttachPointOffset;

	UPROPERTY(EditDefaultsOnly, Category = "Trap")
	USceneComponent* SpreadingOilNiagaraAttachPointOffset;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> RingOriginalMaterialFirstElement;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> RingOriginalMaterialSecondElement;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> RopeAndMeshOriginalMaterialFirstElement;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> RopeMesh;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> RingMesh;

	UPROPERTY(EditAnywhere, Category = "TrapEffect")
	TSubclassOf<class AOil> OilEffect;

	UPROPERTY(EditAnywhere, Category = "TrapEffect")
	TObjectPtr<class UMaterial> OilMaterial;

	UPROPERTY(EditAnywhere, Category = "TrapEffect")
	TObjectPtr<class UNiagaraComponent> NiagaraComponentFallingOil;

	UPROPERTY(EditAnywhere, Category = "TrapEffect")
	TObjectPtr<class UNiagaraComponent> NiagaraComponentSpreadingOil;

	UPROPERTY(EditAnywhere, Category = "TrapEffect")
	TObjectPtr<class UNiagaraSystem> NiagaraSystemFallingOil;

	UPROPERTY(EditAnywhere, Category = "TrapEffect")
	TObjectPtr<class UNiagaraSystem> NiagaraSystemSpreadingOil;

	UPROPERTY(EditAnywhere, Category = "TrapEffect")
	FSoftObjectPath FallingOilPath;

	UPROPERTY(EditAnywhere, Category = "TrapEffect")
	FSoftObjectPath SpreadingOilPath;
	// WPQKFWHA
	uint8 bCanActivate : 1;

	FTimerHandle CheckOilActorVelocityTimerHandle;

	FTransform ReInstallTransform;

	TSharedPtr<struct FStreamableHandle> AsyncLoadHandle1;
	TSharedPtr<struct FStreamableHandle> AsyncLoadHandle2;
};
