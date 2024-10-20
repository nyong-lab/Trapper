// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Trap/TrapBase.h"
#include "GunpowderBarrelTrap.generated.h"

/// <summary>
/// 화약통 함정
/// </summary>
UCLASS()
class TRAPPERPROJECT_API AGunpowderBarrelTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	AGunpowderBarrelTrap();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// WPQKFWHA
		// WPQKFWHA
		// 	// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA// WPQKFWHA
		// WPQKFWHA
		// 	// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA// WPQKFWHA
		// WPQKFWHA
		// 	// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
		// WPQKFWHA
public:
	virtual void Demolish() override;
	virtual void ReInstall() override;
	virtual void OnAttackedByBoss() override;
	FORCEINLINE uint8 IsTriggered1P() const { return bTriggered1P; }
	FORCEINLINE uint8 IsTriggeredExternal() const { return bTriggeredExternal; }

protected:
	UFUNCTION()
	void OnOverlapBeginExplosionTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	UFUNCTION()
	void OnOverlapEndExplosionTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnOverlapBeginTrap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	/*UFUNCTION()
	void OnEffectFinished(class UParticleSystemComponent* ParticleSystem);*/

	UFUNCTION()
	void OnNiagaraSystemFinished(UNiagaraComponent* FinishedComponent);

private:
	void ActivateTrap();
	void WreckageActivation();
	void ChangeOriginalMaterial();
	void NiagaraDeactivateCallback();
	void OverlappingBarrelsExplosionCallback();
	void NextTickDestory();

private:

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> ActivateSound;

	UPROPERTY(EditAnywhere, Category = "TrapEffect")
	TObjectPtr<class UNiagaraComponent> NiagaraComponentExplosion;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class USphereComponent> ExplosionTrigger;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> OriginalMaterial;

	TSet<TObjectPtr<AActor>> OverlappingActors;

	TSet<TObjectPtr<AActor>> OverlappingBarrels;
	TSet<TObjectPtr<AActor>> ExplosionBarrels;

	FTimerHandle DebugDrawTimerHandle;
	FTimerHandle ExplosionTimerHandle;

	uint8 bExplodedBarrel : 1;

	TWeakObjectPtr<AController> TriggeredByController;

	uint8 bSafeRangedFor = false;

	uint8 bTriggered1P : 1 = false;
	uint8 bTriggeredExternal : 1 = false;

	TArray<TObjectPtr<AActor>> SafeAddActorArray;
	TArray<TObjectPtr<AActor>> SafeRemoveActorArray;
	TArray<TObjectPtr<AActor>> SafeAddBarrelArray;
	TArray<TObjectPtr<AActor>> SafeRemoveBarrelArray;

	UPROPERTY(EditAnywhere, Category = "GC")
	TSubclassOf<class AActor> GunpowderBarrelGC;

	uint8 bSafeExplosionBarrelsCallback : 1 = false;
	uint8 bSafeExplosionBarrelsCallbackReal : 1 = false;


	/// RPC --------------------------------------------------------
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCActivateEffect();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSpawnGC();

};
