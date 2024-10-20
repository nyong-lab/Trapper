// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Trap/TrapBase.h"
#include "PlankTrap.generated.h"

/// <summary>
/// 판자 함정
/// 시뮬레이션을 서버에서 돌리는데 클라이언트에서 뚝뚝 끊겨보인다
/// 보간을 해서 부드럽게 만들어야하는데 일단 NetUpdateFrequency 를 500으로 올림
/// </summary>
UCLASS()
class TRAPPERPROJECT_API APlankTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	APlankTrap();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Demolish() override;
	virtual void ReInstall() override;
	FORCEINLINE uint8 IsTriggered1P() const { return bTriggered1P; }

protected:
	UFUNCTION()
	void OnHitArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	void CheckPlankVelocity();
	void DestroyTrap();
	void WreckageActivation();
	void ChangeOriginalMaterial();
	void PlaySound();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> ActivateSound;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> RingAndRopeOriginalMaterialFirstElement;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> MeshOriginalMaterialFirstElement;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> RopeMesh;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> RingMesh;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> ActivateNiagara;

	// 공격 받은 액터 컨테이너
	TSet<TObjectPtr<AActor>> OnHitActors;

	// 시뮬레이션 중인지
	uint8 bIsSimulate : 1;

	// 타이머 핸들
	FTimerHandle CheckPlankVelocityTimerHandle;
	FTimerHandle DestroyTrapTimerHandle;

	// 트랩을 삭제하는 함수 호출 시간
	UPROPERTY(EditAnywhere, Category = "Trap")
	float DestroyTrapTime;

	// 판자의 속력을 체크하는 함수 호출 시간
	float CheckPlankVelocityTime;

	// 물체가 데미지를 줄 수 있는 최소 속력의 사이즈 값 (vector.size)
	float StopDamageOfVelocity;

	FTransform ReInstallTransform;

	// Sound
	uint8 bPlayHitSound : 1;

	TWeakObjectPtr<AController> TriggeredByController;

	uint8 bTriggered1P : 1 = false;

	uint8 bActivateNiagara : 1 = false;

	UPROPERTY(EditAnywhere, Category = "GC")
	TSubclassOf<class AActor> PlankGC;
	// WPQKFWHA
	/// RPC --------------------------------------------------------
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCWreckActivation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSetCollisionResponseOfVelocity();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSetCollisionResponseOfSimulating();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCActivateNiagara(FVector ImpactPoint);
};
