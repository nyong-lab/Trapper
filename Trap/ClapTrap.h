// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Trap/TrapBase.h"
#include "ClapTrap.generated.h"

// WPQKFWHA

/// <summary>
/// 박수 함정
/// </summary>
UCLASS()
class TRAPPERPROJECT_API AClapTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	AClapTrap();
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
		// WPQKFWHA
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void ReInstall() override;
	virtual void OnAttackedByBoss() override;
	virtual void Demolish() override;

public:
	FORCEINLINE AClapTrap* GetOtherClap() const { return OtherClap; }
	void SetOtherClapInteraction(ATrapBase* OtherTrap);
	void SetOtherClap(ATrapBase* OtherTrap);
	FORCEINLINE bool IsMagneticTriggerActivated() const { return bMagneticTriggerActivated; }
	FORCEINLINE void SetActiveTrap(bool Value) { bIsActiveTrap = Value; }
	void SetWeckageOnTimer();
	void SetMeshBlockOnTimer();
	AActor* GetActivatedTriggerToPlayer() const { return ActivatedTriggerToPlayer; }
	void AddOnHitActor(AActor* Actor);

	void ConnectionOtherClap(FVector OtherClapLocation);

	/// RPC --------------------------------------------------------
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSetTrapEnable();

protected:
	UFUNCTION()
	void OnHitMagneticArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnOverlapBeginCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	void ActivateTrap();
	void PlayMagneticSound();
	void ClearTriggerTimer();
	void ChangeOriginalMaterial();
	void WreckageActivation();
	void ClearTimer();
	void CheckAndResetKillCountCollaborationForClapTrap();
	void ClearTriggerTimerCallback();
	void ReActivatedCallback();
	void SetWeckageOnTimerCallback();
	void SetMeshBlockOnTimerCallback();

private:
	FTimerHandle ReActivatedTimerHandle;
	uint8 bReActivated : 1 = true;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<class UNiagaraComponent> NiagaraComponentSand;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<class UNiagaraComponent> NiagaraComponentConnectBlue;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<class UNiagaraComponent> NiagaraComponentConnectRed;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> RedNiagara;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> BlueNiagara;
	 
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> MagneticSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> ActivateSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> BumpSound;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterial> MaterialBlue;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterial> MaterialRed;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> ActivateNiagara;

	// 마그네틱 화살로 활성화 됐는지 여부
	uint8 bMagneticTriggerActivated : 1;

	// 트랩이 활성화 됐는지
	uint8 bIsActiveTrap : 1;

	uint8 bIsBump : 1;

	uint8 bMagneticInteraction : 1;

	uint8 bIsOnAttackedByBoss : 1 = false;

	// 나와 함께 동작 할 다른 박수 함정
	UPROPERTY(ReplicatedUsing = OnRep_OtherClap)
	TObjectPtr<AClapTrap> OtherClap;

	UFUNCTION()
	void OnRep_OtherClap();

	// 공격 받은 액터 컨테이너
	TSet<TObjectPtr<AActor>> OnHitActors;

	// 트랩을 활성화 시킨 플레이어
	TObjectPtr<AActor> ActivatedTriggerToPlayer;

	FTimerHandle ClearTriggerTimerHandle;

	TObjectPtr<class UMaterialInterface> OriginalMaterial1;
	TObjectPtr<class UMaterialInterface> OriginalMaterial2;
	TObjectPtr<class UMaterialInterface> OriginalMaterial3;
	TObjectPtr<class UMaterialInterface> OriginalMaterial4;

	uint8 bIsReInstalled : 1;

	FVector WreckageLocation;

	TWeakObjectPtr<AController> TriggeredByController;

	UPROPERTY(EditAnywhere, Category = "GC")
	TSubclassOf<class AActor> ClapGC;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSpawnGC();

	/// RPC -------------------------------------
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSpawnNiagaraSystem(bool IsServer, const FVector& ImpactPoint);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCChangeMaterial(bool IsServer);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCChangeOriginalMaterial();
};
