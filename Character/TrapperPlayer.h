// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "InputActionValue.h"
#include "Math/Vector.h"
#include "Components/TimelineComponent.h"

#include "TrapperPlayer.generated.h"

//DECLARE_DELEGATE_OneParam(FAffectByTrapDelegate, class ATrapBase* /*InTrap*/);
//USTRUCT(BlueprintType)
//struct FAffectByTrapDelegateWapper
//{
//	GENERATED_BODY()
//	FAffectByTrapDelegateWapper() {}
//	FAffectByTrapDelegateWapper(const FAffectByTrapDelegate& InTrapDelegate) : TrapDelegate(InTrapDelegate) {}
//	
//	FAffectByTrapDelegate TrapDelegate;
//};


UENUM(BlueprintType)
enum class ECharacterControlType : uint8
{
	Moving			UMETA(DisplayName = "Moving"),
	MovingAlt		UMETA(DisplayName = "MovingAlt"),
	Idle			UMETA(DisplayName = "Idle"),
	IdleAlt			UMETA(DisplayName = "IdleAlt"),
	Drawing			UMETA(DisplayName = "Drawing"),
	MagneticMoving	UMETA(DisplayName = "MagneticMoving"),
};

UCLASS()
class TRAPPERPROJECT_API ATrapperPlayer : public ACharacter
{
	GENERATED_BODY()

#pragma region Main & Override

public:
	ATrapperPlayer(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Landed(const FHitResult& Hit) override;

public:
	TObjectPtr<class ATrapperGameState> TrapperGameState;
	TObjectPtr<class ATrapperPlayerController> PlayerController;

#pragma endregion Main & Override

#pragma region Input Action Binding

public:
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	virtual void Jump() override;
	void Interact(const FInputActionValue& Value);

	void BackOrOnSystemMenu();
	void DebugActive();

	void FClickStarted(const FInputActionValue& Value);
	void FClickOngoing(const FInputActionValue& Value);
	void FClickCanceled(const FInputActionValue& Value);
	void FClickCompleted(const FInputActionValue& Value);

	void CameraFixStart(const FInputActionValue& Value);
	void CameraFixEnd(const FInputActionValue& Value);

	/// Debug Function ---------------------------------------------

	void GoMaintenance1(const FInputActionValue& Value);
	void GoMaintenance2(const FInputActionValue& Value);
	void GoMaintenance3(const FInputActionValue& Value);
	void GoMaintenance4(const FInputActionValue& Value);
	void GoMaintenance5(const FInputActionValue& Value);
	void GoMaintenance6(const FInputActionValue& Value);
	void AfterBoss(const FInputActionValue& Value);
	void GameRestart(const FInputActionValue& Value);
	void AddBone(const FInputActionValue& Value);
	void Invincibility(const FInputActionValue& Value);
	void MonsterKill(const FInputActionValue& Value);
	void SelfKill(const FInputActionValue& Value);

	void AllMonsterKill();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSelfKill();

	UFUNCTION(Server, Reliable)
	void ServerSelfKill();

	UFUNCTION(Server, Reliable)
	void ServerRequiredCheat(int32 Code);

	/// ------------------------------------------------------------


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* FClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* CameraFixAction;

	/// <summary>
	/// System Action
	/// </summary>

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-System")
	class UInputAction* BackAction;

	/// <summary>
	/// Debug Action
	/// </summary>

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* DebugActiveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* RestartAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* ToMaintenance1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* ToMaintenance2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* ToMaintenance3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* ToMaintenance4Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* ToMaintenance5Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* ToMaintenance6Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* AfterBossAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* AddBoneAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* InvincibilityAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* KillMonstersAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input-Debug")
	class UInputAction* KillSelfAction;

private:
	uint8 bIsDebugActive : 1;

#pragma endregion Input Action Binding

#pragma region Move

public:
	void CharacterControlTypeCheck();
	void SetCharacterControlData(ECharacterControlType type);
	void TurnInterp();
	float GetLookRotation();
	FRotator GetAimOffset() const;

	UFUNCTION(Server, Unreliable)
	void ServerRPCChangeControlOptions(ECharacterControlType type);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastChangeControlOptions(ECharacterControlType type);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	float TurnSmoothFactor = 0.05f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Control")
	ECharacterControlType ControlState = ECharacterControlType::Idle;

	void AdjustMoveSpeed();
	void AdjustMagneticMoveSpeed();
	void AdjustMagneticMoveAfterSpeed();

private:
	uint8 bIsAltPressed : 1;
	uint8 bCanCameraTurn : 1;
	FRotator InitialRotationRate;
	float LookSmoothFactor = 0.1f;
	float PreviousAngle = 0.f;
	float OriginalBrakingDecelerationFalling = 0.f;

#pragma endregion Move

#pragma region Mesh

public:
	UPROPERTY(EditDefaultsOnly, Category = "Character Mesh")
	TObjectPtr<class USkeletalMesh> FirstCharacterMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Character Mesh")
	TObjectPtr<class USkeletalMesh> SecondCharacterMesh;

private:
	void SetCharacterMesh();

#pragma endregion Mesh

#pragma region Character State

public:
	UPROPERTY()
	TObjectPtr<class UDataTable> CharacterData;

	void ApplyCharacterData(uint8 Type);

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	float ServerTakeDamage(float Damage, FDamageEvent const& DamageEvent, AActor* DamageCauser);
	bool PlayDamageTypeAnimation(float Damage, FDamageEvent const& DamageEvent);

	UFUNCTION(Client, Reliable)
	void ClientRPCTakeDamage(AActor* DamageCauser, bool bIsMonster);

	UFUNCTION(Server, Reliable) void ServerRPCCharacterDamaged(float Damage, FDamageEvent const& DamageEvent, AActor* DamageCauser);
	UFUNCTION() void DamagedEnd(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(NetMulticast, Reliable) void MulticastRPCCharacterAlive(float Damage, FDamageEvent const& DamageEvent, AActor* DamageCauser);
	UFUNCTION(NetMulticast, Reliable) void MulticastRPCCharacterDeath();

	UFUNCTION(Server, Reliable) void ServerRPCCharacterRespawn();
	UFUNCTION(NetMulticast, Reliable) void MulticastCharacterRespawn();
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Character State") float MaxHP = 200.f;
	UPROPERTY(ReplicatedUsing = "OnRep_CurrentHP", VisibleAnywhere, Category = "Character State")
	float CurrentHP;
	UFUNCTION() void OnRep_CurrentHP();

	UPROPERTY(EditDefaultsOnly, Category = "Character State") FVector RespawnPoint;

	// Getter & Setter 
	bool GetDeadState() { return IsDead; }
	void SetControlState(ECharacterControlType type) { ControlState = type; }
	ECharacterControlType GetControlState() const { return ControlState; }
	void AdjustPlayerMaxHealth();

	UFUNCTION(Server, Reliable)
	void ServerRPCRecoveryHealth();

	void SetInvincibility();
	UFUNCTION() void EndInvincibilityTime();
	UFUNCTION() void CheckHealthCoolTime();
	void SetRecoveryHealth();
	UFUNCTION() void RecoveryHealth();
private:
	float HealthCoolTime;
	float HealthRecoveryAmount;
	float RecoveryDelayTime;

	float InvincibilityTime;
	uint8 bIsInvincibility : 1;

	FTimerHandle InvincibilityHandle;
	FTimerHandle RecoveryHandle;
	FTimerHandle HealthCoolTimeHandle;

	uint8 IsDamaged : 1;
	uint8 IsDead : 1;

#pragma endregion Character State

#pragma region Magnetic Interactive

public:
	void FindMagneticPillar();

	// Casting
	UFUNCTION(Server, Reliable) void ServerPlayCastAnimation();
	UFUNCTION(NetMulticast, Reliable) void MulticastPlayCastAnimation();
	void PlayCastAnimation();
	UFUNCTION() void CastEnd(UAnimMontage* Montage, bool bInterrupted);

	// Moving
	UFUNCTION(Server, Reliable) void ServerWantsToMagneticMove();
	UFUNCTION(NetMulticast, Reliable) void MulticastWantsToMagneticMove();
	void WantsToMagneticMove();

	// 플레이어 관련 처리가 필요할 때 사용
	void StartManeticMovingSetting();
	void FinishMagneticMovingSetting();

	bool IsServerProxy();

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UNiagaraSystem> SpeedLineEffect;

	void SpawnSpeedLineEffect(bool Value);

#pragma endregion Magnetic Interactive

#pragma region Interactive

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interact")
	float InteractDistance = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interact")
	float InteractRadius = 500.f;

	UPROPERTY(ReplicatedUsing = OnRep_SkipGauge)
	float SkipGauge;

	float SkipAccumulatelTime;
	float SkipTime = 2.f;

	UFUNCTION() void OnRep_SkipGauge();

	void CalculateSkipGauge();
	bool SkipCurrentStage();

	UFUNCTION(Server, Unreliable)
	void ServerRPCSkipGaugeChange(float Value);

	UFUNCTION(Server, Reliable)
	void ServerRPCSkip();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCPlayInstallAnim();

	UFUNCTION(Server, Unreliable)
	void ServerRPCPlayInstallAnim();

	FTimerHandle CheckPlayerSightTimer;
	void CheckPlayerSight();

#pragma endregion Interactive

#pragma region Item

public:
	void FindMagneticItem();

	void AddBoneItem(int32 Count = 1);
	bool UseBoneItem(int32 Count);

	UFUNCTION(Server, Reliable)
	void ServerRPCUseBoneItem(int32 Count);

	UFUNCTION(Server, Reliable)
	void ServerRPCAddBoneItem(int32 Count);

	UFUNCTION()
	void OnRep_BoneItemBox();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	int32 MaxItemCount = 9999;

	UPROPERTY(ReplicatedUsing = OnRep_BoneItemBox, EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 BoneItemBox = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic")
	float MagneticPullRange = 500.f;

#pragma endregion Item

#pragma region Trap

public:
	void EnterInstallationMode();
	void ReleaseInstallationMode();
	FORCEINLINE uint8 IsInstallationMode() const { return bInstallationMode; }
	FORCEINLINE uint8 IsPreviewStartAnim() const { return bPreviewStartAnim; }
	uint8 IsTrapSelectMode() const;
	FORCEINLINE void SetPreviewStartAnim(bool bIsAnim) { bPreviewStartAnim = bIsAnim; }
	void StartInstallAnim();
	void EndInstallAnim();

	void UpgradeTrapInstallCost(float CostIcreaseDecreaseRate);
	void UpgradeTrapDamage(float Rate);

	FORCEINLINE uint8 GetOverlapOilCount() const { return OverlapOilCount; }
	FORCEINLINE void OverlapOilCountUp() { ++OverlapOilCount; }
	FORCEINLINE void OverlapOilCountDown() { --OverlapOilCount; }
	FORCEINLINE bool Is1Player() const { return b1Player; }

	bool IsInstallSpikeTrap() const;
	bool IsInstallGunpowderBarrelTrap() const;

public:
	UFUNCTION(Client, Reliable)
	void ClientRPCUpdateTrapHudNum(ETrapType Type, bool bInstall);

private:
	void EndInstallAnimTimer();
private:
	uint8 bInstallationMode : 1;
	uint8 bPreviewStartAnim : 1;
	uint8 OverlapOilCount = 0;
	uint8 b1Player : 1 = false;

	uint8 MaxSpikeNum = 3;
	uint8 MaxGunpowderBarrelNum = 5;
	uint8 SpikeNum = 0;
	uint8 GunpowderBarrelNum = 0;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCUpgradeTrapInstallCost(float Rate);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCUpgradeTrapDamage(float Rate);

	FTimerHandle AnimHandle;

#pragma endregion Trap

#pragma region Bow

public:
	UFUNCTION() void SetCameraAim(float Value);
	UFUNCTION() void SetCameraShoot(float Value);
	void BlockBow();
	void UnblockBow();

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float AimFov = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float ShootFov = 77.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector AimCameraArmOffset = FVector(230, 80, 70);

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	UCurveFloat* BowCameraCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	UCurveFloat* BowCameraCurveShoot;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float BowCameraLerpDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float BowCameraLerpDurationReverse = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float BowCameraLerpDurationShoot = 0.1f;

	FTimeline BowCameraTimeLine;
	FTimeline BowCameraShootTimeLine;

private:
	float InitialFOV;
	FVector InitialCameraArmOffset;

#pragma endregion Bow

#pragma region UI

public:
	void CreatePlayerHUD();
	bool IsPingSelectMode() const;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitFeedbackHUD(bool bHeadShoot, bool bFullCharged);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowKillFeedbackHUD();

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UUserWidget> PlayerHudClass;

	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	class UPlayerHUD* PlayerHud;

#pragma endregion UI

#pragma region Cursor
public:
	void ShowCursor();
	void HideCursor();
	void SetCursorToCenter();
	FVector2D GetCursorPosition();
	float GetViewportScale();

#pragma endregion Cursor

#pragma region Camera

public:
	void AttachCameraToSpringArm();

	FVector InitialSocketOffset;
	float InitialArmLength;

#pragma endregion Camera

#pragma region Components

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UNiagaraComponent> SpeedLineEffectComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UBowMachanics> BowMechanics;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UTrapMechanics> TrapMechanics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	TObjectPtr<class UPingMechanics> PingMechanics;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UMapMechanics> MapMechanics;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UMapMarkerComponent> MapMarkerComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UTrapperPlayerMovementComponent> Movement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UAudioComponent> AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UAudioComponent> AwakenAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UUpgradeMechanics> UpgradeMechanics;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UIndicatorComponent> IndicatorComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UHitIndicatorComponent> HitIndicatorComponent;

#pragma endregion Components

#pragma region Animation

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<class UAnimMontage> DamagedAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<class UAnimMontage> DeathAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<class UAnimMontage> MagneticMoveMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<class UAnimMontage> TrapInstallMontage;

#pragma endregion Animation

#pragma region Sound
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundWave> InteractionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundBase> AwakenStartSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundBase> AwakenLoopSound;

#pragma endregion Sound

#pragma region Option

public:
	FORCEINLINE void SetSensivityScale(const float MouseValue, const float AimValue) { MouseSensivityScale = MouseValue; AimSensivityScale = AimValue; }

private:
	float MouseSensivityScale = 0.5f;
	float AimSensivityScale = 0.5f;

#pragma endregion Option

#pragma region Awaken
public:
	void S2SSetAwaken(bool bIsServer);

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UNiagaraSystem> AwakeStartEffect1P;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UNiagaraSystem> AwakeStartEffect2P;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UNiagaraSystem> AwakeEffect1P;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<class UNiagaraSystem> AwakeEffect2P;


private:
	bool bIsAwaken = false;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSetAwaken(bool bIsServer);
	void MulticastRPCSetAwaken_Implementation(bool bIsServer);

public:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCStopAwakenSound();

#pragma endregion Awaken
};

