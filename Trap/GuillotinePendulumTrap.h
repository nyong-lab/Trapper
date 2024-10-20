// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Trap/TrapBase.h"
#include "Components/TimelineComponent.h"
#include "GuillotinePendulumTrap.generated.h"

/// <summary>
/// 기요틴 진자 함정
/// </summary>
UCLASS()
class TRAPPERPROJECT_API AGuillotinePendulumTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	AGuillotinePendulumTrap();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

public:
	virtual void Demolish() override;
	virtual void OnAttackedByBoss() override;
	FORCEINLINE bool IsMagneticTriggerActivated() const { return bMagneticTriggerActivated; }
	void SetActiveTrap(bool Value/*활성화 여부*/, bool FirstMagneticTriggerDirection /*첫 번째 마그네틱 트리거 방향인지*/);
	void SetEnableCanAttack();
	void TickSwing();
	FORCEINLINE AActor* GetActivatingPlayer() const { return ActivatingPlayer; }
	void AllMaterialBlue();
	void AllMaterialRed();
	bool IsCanInstall() const;
	void SetAllMaterialOriginal();
	void DemolishGuillotine();
	void SetMapmaker(bool IsAlive);

	FORCEINLINE UStaticMeshComponent* GetMesh() const { return Mesh; }
	FORCEINLINE UStaticMeshComponent* GetBeam() const { return Beam; }
	FORCEINLINE UStaticMeshComponent* GetPanel1() const { return Panel1; }
	FORCEINLINE UStaticMeshComponent* GetPanel2() const { return Panel2; }

protected:
	UFUNCTION()
	void OnHitArrow(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnOverapCharacter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

private:
	void CalculatePendulum(float& NewRoll, float& SinValue, float& DampingFactor);
	void CalculateDamage(const float& NewRoll);
	void OutHitActor(AActor* OtherActor);
	void SetActivatingPlayer(AActor* OtherActor);
	void SetMeshMaterialBlue(bool Value);
	void SetPanel1MaterialBlue(bool Value);
	void SetPanel2MaterialBlue(bool Value);
	void ClearTriggerTimer();
	void PlayMagneticSound();
	void MoveSoundCallback();
	void ReAttackTimeCallback();
	TMap<AActor*, FTimerHandle> HitActorTimers;

	void ClearTriggerTimerCallback();

public:
	// For Niagara
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara")
	uint8 bNiagaraDeactivate : 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara")
	TObjectPtr<class UNiagaraComponent> NiagaraComponentSand;

private:
	TObjectPtr<class USoundWave> CurrentHitSound;
	TObjectPtr<class USoundWave> CurrentMoveSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> MagneticSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> MoveSound1;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> MoveSound2;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> HitSound1;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> HitSound2;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> HitSound3;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> Panel1;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> Panel2;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> Beam;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterial> MaterialBlue;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterial> MaterialRed;

	UPROPERTY(EditAnywhere, Category = "TrapEffect")
	TObjectPtr<class UNiagaraSystem> SandNiagara;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class USphereComponent> TriggerSphere;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> RedNiagara;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> BlueNiagara;

	/*UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraComponent> ActivateNiagara;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UNiagaraSystem> ActivateNiagara1;*/

	// 트랩 재활성화 시간 (쿨타임)
	UPROPERTY(EditAnywhere, Category = "Trap")
	float ReEnableTime;

	// 데미지를 입은 액터가 다시 데미지를 입을 수 있는 시간
	UPROPERTY(EditAnywhere, Category = "Trap")
	float ReAttackTime;

	// 자성화살로 활성화 되었는지
	uint8 bMagneticTriggerActivated : 1;

	uint8 bMeshMagneticTrigger : 1;

	uint8 bPanel1MagneticTrigger : 1;

	uint8 bPanel2MagneticTrigger : 1;

	TObjectPtr<AActor> MeshMagneticTriggerActivatingPlayer;
	TObjectPtr<AActor> Panel1MagneticTriggerActivatingPlayer;
	TObjectPtr<AActor> Panel2MagneticTriggerActivatingPlayer;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> MeshOriginalMaterialElement0;
	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> MeshOriginalMaterialElement1;
	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> MeshOriginalMaterialElement2;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> PanelOriginalMaterialElement0;
	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> PanelOriginalMaterialElement1;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> BeamOriginalMaterialElement0;
	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterialInterface> BeamOriginalMaterialElement1;

	// 트랩이 활성화 됐는지
	uint8 bIsActiveTrap : 1;

	// 트랩이 활성화된 누적 시간
	float ActivateTrapTime;

	// 재발동 타이머 핸들
	FTimerHandle ReEnableTimerHandle;

	// 재공격 타이머 핸들
	FTimerHandle ClearTriggerTimerHandle;

	// 스냅 환경 액터 첫 번째 마그네틱 트리거 방향인지
	uint8 bFirstMagneticTriggerDirection : 1;

	// 함정이 발동되면 타겟 방향으로 한번 러프 시킨다.
	uint8 bIsLerpingToTarget : 1;

	// 러프가 완료 됐는지
	uint8 bLerpCompleted : 1;

	UINT8 bFirstRestrictionActivation : 1;

	// 진자운동에서 사용하는 이전 Roll 값
	float LastRoll;

	// 진자운동 시작 각도
	float MaxRoll;

	float InitialDamage = 0.f;

	// 공격 받은 액터 컨테이너
	TSet<TObjectPtr<AActor>> OnHitActors;

	// 트리거를 활성화 시킨 플레이어 저장
	TObjectPtr<AActor> ActivatingPlayer;

	TSet<TObjectPtr<AActor>> OverlappingTraps;

	FVector TrapPrevLocation;
	FVector TrapFrameNormal;
	float LaunchSpeed;

	FTransform InitialMeshTransform;

	// For Sound
	float PreviousRollForDetermineSwingDirection;
	float CurrentRollForDetermineSwingDirection;
	uint8 bSoundRight : 1;
	uint8 bPlayMoveSound : 1;
	float Volume;
	// WPQKFWHA
	uint8 bSoundChangeHit2 : 1;
	uint8 bSoundChangeHit1 : 1;

	TWeakObjectPtr<AController> TriggeredByController;
	uint8 bCheckKillCount : 1 = false;

	UPROPERTY(EditAnywhere, Category = "GC")
	TSubclassOf<class AActor> BeamGC;

	UPROPERTY(EditAnywhere, Category = "GC")
	TSubclassOf<class AActor> MeshGC;

	UPROPERTY(EditAnywhere, Category = "GC")
	TSubclassOf<class AActor> BeamSmallGC;

	UPROPERTY(EditAnywhere, Category = "GC")
	TSubclassOf<class AActor> MeshSmallGC;
	

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSpawnGC();

	/// RPC --------------------------------------------------------
public:
	// 환경 액터에서도 사용하기에 public
	// 서버 클라 모두 진자운동 연산
	// 충돌 처리는 서버에서만 
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSetTrapActive(bool Value1, bool Value2);

private:

	// 이펙트 멀티캐스트
	/*UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCActivateEffectt(const FVector& ImpactPoint);*/

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSetActivingPlayer(AActor* OtherActor);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCSpawnNiagaraSystem(bool IsServer, const FVector& ImpactPoint);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCSetMeshMaterialBlue(bool Value);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCSetPanel1MaterialBlue(bool Value);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCSetPanel2MaterialBlue(bool Value);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCSetMeshMaterialOriginal();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCSetPanel1MaterialOriginal(bool Value);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCSetAllMaterialOriginal();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCPlayHitSound();
};


//ActivateTrapTime += GetWorld()->GetDeltaSeconds();
//
//// 현재 회전값을 기준으로 진동 시작
//float CurrentRoll = GetActorRotation().Roll;
//
//// 최대 회전값 설정
//// Sin 함수를 이용한 진동값 계산
//float TargetRoll = 0.f;
//float SinValue = 0.f;
//if (InitialRoll > 0.f)
//{
//	TargetRoll = 70.f;
//	SinValue = FMath::Sin(ActivateTrapTime * 3) * (TargetRoll - InitialRoll);
//}
//else if (InitialRoll < 0.f)
//{
//	TargetRoll = -70.f;
//	SinValue = FMath::Sin(ActivateTrapTime * 3) * (TargetRoll - InitialRoll);
//}
//else
//{
//	SinValue = FMath::Sin(ActivateTrapTime * 3);
//}


// 첫 번째 마그네틱 트리거 방향이 아니라면 반대로 스윙
	/*if (!bFirstMagneticTriggerDirection)
	{
		SinValue *= -1;
	}*/