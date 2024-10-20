// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Trap/TrapBase.h"
#include "BearTrap.generated.h"

UENUM()
enum class ChangeMeshDirection : uint8
{
	Left = 0,
	Right,
	All
};

UENUM()
enum class ChangeMeshMaterial : uint8
{
	Server,
	Client,
	Original
};

/// <summary>
/// 찰코 함정
/// </summary>
UCLASS()
class TRAPPERPROJECT_API ABearTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	ABearTrap();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Demolish() override;

public:
	virtual void ReInstall() override;

public:
	void MagneticTriggerControl(bool bIsOn);
	void DestroyHandle();
	virtual void OnAttackedByBoss() override;

private:
	UFUNCTION(Server, Reliable)
	void ServerRPCMagneticTriggerControl(bool bIsOn);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCMagneticTriggerControl(bool bIsOn);

protected:
	UFUNCTION()
	void OnOverlapBeginCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	UFUNCTION()
	void OnOverlapEndCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnHitMagneticArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	void ActivateTrap();
	void ClearTriggerTimer();
	void WreckageActivation();
	void ChangeOriginalMaterial();
	void PlayMagneticSound();
	void ClearTriggerTimerCallback();
	void ClearTutorialTrap();
	void ReActivatedCallback();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> ActivateSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> MagneticSound;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UBoxComponent> MagneticTrigger1;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UBoxComponent> MagneticTrigger2;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> RedNiagara;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> BlueNiagara;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> ActivateNiagara;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> LeftMesh;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> RightMesh;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> CentralMesh;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class USkeletalMeshComponent> SkeletalMesh;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UAnimationAsset> CloseAnimation;

	// 오버랩 중인 액터들
	UPROPERTY()
	TSet<TObjectPtr<AActor>> OverlappingActors;

	// 트리거를 발동시킨 플레이어 저장

	TObjectPtr<AActor> FirstMagneticTriggerActivatingPlayer;
	TObjectPtr<AActor> SecondMagneticTriggerActivatingPlayer;

	// 메쉬의 오리지널 머테리얼 저장
	TObjectPtr<class UMaterialInterface> OriginalMaterialFirstElement;
	TObjectPtr<class UMaterialInterface> OriginalMaterialSecondElement;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterial> MaterialBlue;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterial> MaterialRed;
	
	uint8 bFirstActivatedTriggerIsServerPlayer : 1 = false;
	FTimerHandle ReActivatedTimerHandle;
	uint8 bReActivated : 1 = true;
	FTimerHandle ClearTriggerTimerHandle;

	uint8 bSafeRangedFor = false;
	// WPQKFWHA// WPQKFWHA// WPQKFWHA// WPQKFWHA// WPQKFWHA// WPQKFWHA// WPQKFWHA// WPQKFWHA
	TArray<TObjectPtr<AActor>> SafeAddArray;
	TArray<TObjectPtr<AActor>> SafeRemoveArray;

	TWeakObjectPtr<AController> TriggeredByController;

	UPROPERTY(EditAnywhere, Category = "GC")
	TSubclassOf<class AActor> BearGC;

	// WPQKFWHAaasdasd

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSpawnGC();

	/// RPC --------------------------------------------------------
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCActivateEffect();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCChangeMaterial(ChangeMeshDirection Direction, ChangeMeshMaterial Material);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCSpawnNiagaraSystem(const FVector& ImpactPoint, bool IsServer);
};
