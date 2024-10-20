// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Trap/TrapBase.h"
#include "SpearTrap.generated.h"

/**
 *
 */
UCLASS()
class TRAPPERPROJECT_API ASpearTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	ASpearTrap();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

public:
	virtual void Demolish() override;
	virtual void ReInstall() override;
	virtual void OnAttackedByBoss() override;
	void SetMapMarkerSpearWallInitialLocation();
	void ConnectionSpearAndWall();

protected:
	UFUNCTION()
	void OnHitMagneticArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnOverlapBeginCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

private:
	void ThrownSpear();
	void ClearMagneticTimer();
	void PlayMagneticSound();
	void WreckageActivation();
	void ChangeOriginalMaterial();
	void SetWeckageOnTimer();
	void PlayChargeSound(uint8 Count, bool IsSpear, bool IsServer);
	void PlayCompleteChargeSound(bool IsSpear, bool IsServer);
	void PlayLaunchSound();
	void OnSpearBlinkTimer(bool IsServer);
	void OnWallBlinkTimer(bool IsServer);
	void UpdateDecreaseBlink();
	void ClearAllTimer();
	void ClearBlinkTimer();
	void OnBlinkTimer(bool IsServer, bool IsSpear);
	void ClearTriggerTimerCallback();
	void ReActivatedTimerCallback();
	void SetWeckageOnTimerCallback();
	void BlinkTimerCallback();
	void WallBlinkTimerCallback();
	void BlinkChangeMaterialTimerCallback();
	void WallBlinkChangeMaterialTimerCallback();
	void UpdateDecreaseBlinkTimerCallback();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Blinking")
	float InitialBlinkDuration = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Blinking")
    float FinalBlinkDuration = 0.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Blinking")
    float BlinkDecreaseRate = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<class UNiagaraComponent> NiagaraComponentConnectBlue;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<class UNiagaraComponent> NiagaraComponentConnectRed;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<class UStaticMeshComponent> SpearWall;

	UPROPERTY(EditAnywhere, Category = "MapMarker")
	TObjectPtr<class UMapMarkerComponent> MapMarkerSpearWall;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> ActivateSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> MagneticSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> ChargeSound0;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> ChargeSound1;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> ChargeSound2;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> ChargeSound3;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> CompleteChargeSound;

	UPROPERTY(EditAnywhere, Category = "Material")
	TObjectPtr<class UMaterial> MaterialBlue;

	UPROPERTY(EditAnywhere, Category = "Material")
	TObjectPtr<class UMaterial> MaterialRed;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<class UNiagaraSystem> MagneticRedNiagara;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<class UNiagaraSystem> MagneticBlueNiagara;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<class UNiagaraSystem> WindNiagara;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<class UNiagaraComponent> NiagaraComponentChargeNiagara;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> ActivateNiagara1;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> ActivateNiagara2;

	FTimerHandle BlinkChangeMaterialTimerHandle;
	FTimerHandle UpdateDecreaseBlinkTimerHandle;
	FTimerHandle BlinkTimerHandle;
	FTimerHandle ClearTriggerTimerHandle;
	FTimerHandle ReActivatedTimerHandle;

	UMaterialInstanceDynamic* DynamicMaterialInstance = nullptr;

	// 트리거를 발동시킨 플레이어 저장
	TObjectPtr<AActor> SpearMagneticTriggerActivatingPlayer;
	TObjectPtr<AActor> SpearWallMagneticTriggerActivatingPlayer;

	// 메쉬의 오리지널 머테리얼 저장
	TObjectPtr<class UMaterialInterface> SpearOriginalMaterial;
	TObjectPtr<class UMaterialInterface> SpearWallOriginalMaterial;

	float CurrentBlinkDuration = 0.f;

	uint8 bIsActiveTrap : 1 = false;
	FVector WreckageLocation;
	uint8 bReActivated : 1 = true;

	uint8 IncreaseCount = 0;
	uint8 bFirstOnBlinkTimer : 1 = true;

	uint8 bIsServer : 1 = false;

	TSet<TObjectPtr<AActor>> OnHitActors;

	FRotator FromSpearToWall;
	FRotator FromWallToSpear;
	// WPQKFWHA// WPQKFWHA// WPQKFWHA// WPQKFWHA// WPQKFWHA
	UPROPERTY(EditAnywhere)
	float OpacityIncrement = 0.2f;

	TWeakObjectPtr<AController> TriggeredByController;

	UPROPERTY(EditAnywhere, Category = "GC")
	TSubclassOf<class AActor> SpearGC;

	UPROPERTY(EditAnywhere, Category = "GC")
	TSubclassOf<class AActor> WallGC;

	// WPQKFWHA 

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSpawnGC();

	/// RPC --------------------------------------------------------
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSetMapMarkerSpearWallInitialLocation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSpawnNiagaraSystem(bool IsServer, const FVector& ImpactPoint);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSpawnConnectionNiagaraSystem(bool IsServer, bool IsHitSpear, bool IsHitWall);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCChangeSpearMaterial(bool IsServer, uint8 Count);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCChangeSpearWallMaterial(bool IsServer, uint8 Count);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCChangeSpearOriginalMaterial();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCChangeSpearWallOriginalMaterial();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCChangeAllMeshOriginalMaterial();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCActivateTrap(uint8 Count);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCConnectionSpearAndWall();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCPlayChargeSound(uint8 Count, bool IsSpear, bool IsServer);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCPlayCompleteChargeSound(bool IsSpear, bool IsServer);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCInitializeBlink(bool IsServer, bool IsSpear, bool IsActivate);
};