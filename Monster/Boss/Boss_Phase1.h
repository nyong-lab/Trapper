// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/BaseMonster.h"
#include "Boss_Phase1.generated.h"

/**
 *
 */
UCLASS()
class TRAPPERPROJECT_API ABoss_Phase1 : public ABaseMonster
{
	GENERATED_BODY()

public:
	ABoss_Phase1();

protected:
	virtual void BeginPlay() override;

public:
	///--------------------State-------------------------
	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	///------------------------Attck Skills-------------------------
	void SpawnSword();
	void SpawnChairItem();
	void SpawnSwordItem();


	UFUNCTION()
	void OnOverlapBeginTrapSight(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEndTrapSight(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	///----------------------------Montage RPC-----------------------------
	UFUNCTION(Server, Reliable)
	void ServerRPCADCAttack();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCADCAttack();

	UFUNCTION(Server, Reliable)
	void ServerRPCTrapAttack();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCTrapAttack();

	UFUNCTION(Server, Reliable)
	void ServerRPCPhaseChange();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCPhaseChange();

public:
	//Chair
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TSubclassOf<class ABossChair> BossChairClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class ABossChair> BossChair;

	//근거리 공격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TSubclassOf<class ABossSword> BossSwordClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class ABossSword> BossSword;

	//원거리 공격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attact")
	TObjectPtr<class USceneComponent> SwordSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	TSubclassOf<class ABossThrowSword> ThrowSword;

	//Trap Attack Sight    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	TObjectPtr<class UBoxComponent> TrapAttackSight;

	//BossPhase Change
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<class USceneComponent> ChairSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<class USceneComponent> SwordItemSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<class UNiagaraSystem> MoveEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<class USceneComponent> MoveEffectPosition;

	///------------------Montage----------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> AttackADCMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> PhaseChangeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> AttackTrapMontage;

	///Detect Trap
	TSet<TObjectPtr<class ATrapBase>> DetectTrapList;
	FVector DetectTrapLocation;

	TSet<TObjectPtr<class AMagneticPillar>> DetectMagneticList;
	FVector DetectMagneticLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TrapSkillCoolTime;

	uint8 bIsTrapAttackState : 1 = false;
	void TrapAttackState() { bIsTrapAttackState = true; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ADCAttackCoolTime;

	uint8 bIsADCAttackState : 1 = false;
	void ADCAttackState() { bIsADCAttackState = true; }

	///BossPhase Change
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class ABossItem> ChairItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<class ABossItem> SwordItem;

	uint8 bIsPhaseChange : 1 = false;
	FVector BossChangeLocation;

	uint8 bIsChairSpawn : 1 = false;
	uint8 bIsSwordSpawn : 1 = false;
	uint8 bIsSwordHandle : 1 = false;

	TObjectPtr<class ATrapperGameMode> TrapperGameMode;

private:
	FTimerHandle TimerHandle;
	FTimerHandle ADCTimerHandle;

public:
	uint8 bSafeRanged : 1 = false;

};
