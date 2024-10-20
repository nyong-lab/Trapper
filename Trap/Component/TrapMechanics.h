// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Trap/Utillity/TrapParts.h"
#include "Trap/Utillity/TrapInfo.h"
#include "InputActionValue.h"
#include "TrapMechanics.generated.h"

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TRAPPERPROJECT_API UTrapMechanics : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTrapMechanics();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	FORCEINLINE bool IsTrapSelectMode() const { return bSelectMode; }
	void BlockSelectTrap();
	void UnBlockSelectTrap();
	void DemolishReInstallRaycastLoop();
	FORCEINLINE bool IsPlayDemolishRaycast() const { return bPlayDemolishRaycast; }
	void UpgradeCost(float CostIcreaseDecreaseRate);

private:
	/// 멤버 함수 ------------------------------------------------
	// 인풋 맵핑
	void SetupInputComponent();

	// 설치 모드 중에 레이를 쏴서 프리뷰 메쉬를 띄우기 위한 루프
	void PreviewLoop();

	// 설치 모드 돌입
	void EnterInstallationMode();

	// 설치 모드 해제
public:
	void ReleaseInstallationMode();
private:
	void ReleaseInstallationModeToPlace();

	// 선택 모드 인풋
	void EnterSwitchSelectMode();
public:
	void ExitSwitchSelectMode();
private:
	// 선택 모드 돌입
	void EnterSelectMode();

	// 선택 모드 해제
	void ReleaseSelectMode();

	// 프리뷰 메쉬 설정
	void SetPreviewMesh();

	// 프리뷰 메쉬 해제();
	void ReleasePreviewMesh();

	// 프리뷰 메쉬 머테리얼 설정();
	void SetPreviewMeshMaterial(bool bMaterialBlue, class ATrapSnapTrigger* TSTrigger);

	// 프리뷰 메시 회전
	void RotatePreviewMesh(const FInputActionValue& Value);

	//// 프리뷰 메쉬 교체
	//void ChangePreviewMesh();
	//void PermissionChangePreviewMesh();

	// 트랩 설치
	void LeftMouseClick();
	void PlaceTrap();
	bool PermissionPlaceTrap();

	// 트레이스 채널에 해당하는 콜리전 채널 반환
	ECollisionChannel ConvertTraceTypeToCollisionChannel(ETraceTypeQuery TraceChannel);

	// 스냅 트리거박스에 레이가 닿았는지 체크
	bool DetectSnapTriggerBoxes(const FHitResult& HitResult);

	// 기요틴 전용
	bool DetectSnapTriggerBox(const FHitResult& HitResult);

	void CheckSnabGuillotineTrap(FHitResult& HitResult, FQuat& CameraRotation);

	// 오버랩 중복 검사 (설치 가능한지 체크)
	bool CheckForOverlap();

	bool CheckForOverlapSpearWall();

	// 지형 각도에 맞춰서 함정 각도 설정
	void AlignToSurface();

	// 플레이어에게 설치 모드 해제 전달
	void NotifyPlayerInstallationMode() const;

	// 프리뷰메시, 커플 트랜스폼 설정
	void AllPreviewSetTransform();

	// 생성한 트랩 초기화
	void InitializeTrap(ATrapBase* Trap);

	// 천장 트랩인 경우 추가 체크 요소
	bool CheckCellingTrap(FHitResult& HitResult, FVector& StartLocation, FVector& EndLocation, FVector& CameraForwardVector, FCollisionQueryParams& CollisionParams, bool& bHasHit);

	// 스냅 트랩인 경우 추가 체크 요소
	void CheckSnapTrap(FHitResult& HitResult, bool& bDetectSnabTrigger, FQuat& CameraRotation);

	// RangeMesh 숨길지말지 체크. (서버만)
	void CheckServerBroadCastSetHiddenRange(bool IsServer);

	// RangeMesh 트랜스폼 결정
	void CalculateRangeMeshTransform();

	// 기요틴 전용 오버랩 설치 판정
	bool CheckForOverlapExclusivehGuillotine();

	// 함정 철거
	void DemolishTrap();
	void DestroyFocusTrap();

	// 함정 재설치
	void ReInstallTrap();

	void PlaySoundInstallTrap();

	void SafeNormalizeQuat();

	void ClientDestroyPreviweMeshTimerCallback();
	void AnimMulticastTimerCallback();
	void AnimServerTimerCallback();

	void IncreaseTrapHudNum(ETrapType Type, bool bInstall);
	void DecreaseTrapHudNum(ETrapType Type, bool bInstall);

public:
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<class UDataTable> TrapPartsDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<class UDataTable> TrapDataTable;

	/// Input Action ------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> EnterInstallationModeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> ExitInstallationModeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> RealeseInstallationModeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> LeftMouseClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> RotatePreviewMeshAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> DemolishAction;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	//TObjectPtr<class UInputAction> ShowMeTheMoneyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> ShowScoreHUDAction;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> UpdateScoreHUDAction;*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> SpawnMonsterSpikeTrapsAction;

	/// 프리뷰 메쉬 머테리얼 ------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterial> PreviewMaterialBlue;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UMaterial> PreviewMaterialRed;

	///  ------------- Sound  -------------
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* SelectOverSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* SelectDisableSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* InstallFailSound = nullptr;

	/// 박수 경로 체크
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	TSubclassOf<class AActor> CheckClapPath;

	/// 멤버 변수 ------------------------------------------------
protected:
	// 레이 거리 (시작위치 ~ 시작위치 + MaxRayDistance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RayDistance")
	float MaxInstallRayDistance = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RayDistance")
	float MaxDemolishReInstallRayDistance = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RayDistance")
	float SpearLeftRightRayDistance = 3000.f;

private:
	UPROPERTY()
	TObjectPtr<class ATrapperPlayer> Player;

	UPROPERTY()
	TObjectPtr<class ATrapperPlayerController> PlayerController;

	UPROPERTY()
	TObjectPtr<class UCameraComponent> FollowCamera;

	// 트랩 파츠 테이블
	UPROPERTY(VisibleAnywhere, Category = "Trap")
	TArray<FTrapParts> TrapPartsArray;

	TArray<FTrapInfo> TrapDataArray;

	// 프리뷰 메쉬 로테이션
	FTransform PreviewMeshTransform;
	FTransform PreviewCoupleTransform;
	FTransform PreviewRangeTransform;
	FRotator PreviewMeshRotator;


	// bool 값
	uint8 bInstallationMode : 1;
	uint8 bSelectMode : 1;
	uint8 bCanSnap : 1;
	uint8 bResetTrapPartsTableIndex : 1;
	uint8 bCanInstall : 1;
	uint8 bAlignToSurface : 1;
	uint8 bCanSelect : 1;
	uint8 bCanRotationGuillotine : 1;
	uint8 bBlockSelectTrap : 1;
	uint8 bPlaceTrap : 1;
	uint8 bPlayDemolishRaycast : 1;
	uint8 bHitRayCanReInstall : 1;
	uint8 bCanInstallCouple : 1;

	// 트레이스 채널
	ECollisionChannel CollisionChannel = ECollisionChannel::ECC_Visibility;

	// 트랩 스냅 환경 액터
	TObjectPtr<class ATrapSnabEnviromentBase> TrapSnapEnviromentActor;

	UPROPERTY(Replicated)
	TObjectPtr<class ATrapSnapTrigger> TrapSnapTrigger;

	// 타이머 핸들
	FTimerHandle PreviewLoopTimerHandle;
	FTimerHandle ClientDestroyPreviweMeshTimerHandle;
	FTimerHandle DemolishReInstallRaycastLoopTimerHandle;

	// 함정 타입마다 사용자 시점에서 보기 좋게 만들기 위한 위치 오프셋
	FVector GuillotinePendulumOffset = { 0.f,0.f,500.f };

	// 박수 함정 오프셋
	FVector ClapOffset = { 68.f,0.f,0.f };

	// 함정 설치 비용
	int32 ItemCost = 0;

	// 화약통 나이아가라적용 블루프린트 클래스
	TObjectPtr<UClass> LoadedBPGunpowderNiagaraClass;


	// 커플 함정 첫번째 설치 시 임시 저장
	UPROPERTY(Replicated)
	TObjectPtr<class UStaticMeshComponent> PreviewFirstCoupleMeshTemp;

	float CostRate = 1;

private:
	/// 설치 UI 관련 -----------------------------------------------
	FTimerHandle UpdateHoverCheckTimer;
	void CheckOnHovered();
	//UPROPERTY(ReplicatedUsing = OnRep_HoveredTrapType)
	ETrapType HoveredTrapType = ETrapType::None;
	int32 LastItemBoxCount = 0;
	//UFUNCTION()
	//void OnRep_HoveredTrapType();

private:
	/// 리플리케이션 -----------------------------------------------

	UPROPERTY(EditAnywhere, Category = "Trap", ReplicatedUsing = OnRep_PreviewMesh)
	TObjectPtr<class UStaticMeshComponent> PreviewMesh;

	UPROPERTY(EditAnywhere, Category = "Trap", ReplicatedUsing = OnRep_PreviewRange)
	TObjectPtr<class UStaticMeshComponent> PreviewRange;

	UPROPERTY(EditAnywhere, Category = "Trap", ReplicatedUsing = OnRep_PreviewCouple)
	TObjectPtr<class UStaticMeshComponent> PreviewCouple;

	UPROPERTY(EditAnywhere, Category = "Trap", ReplicatedUsing = OnRep_PreviewRangeDecal)
	TObjectPtr<class ADecalActor> PreviewRangeDecal;

	UPROPERTY(EditAnywhere, Category = "Trap", ReplicatedUsing = OnRep_PreviewNiagara)
	TObjectPtr<class AActor> PreviewNiagara;

	UPROPERTY(ReplicatedUsing = OnRep_TrapPartsTableIndex, VisibleAnywhere, Category = "Trap")
	uint8 TrapPartsTableIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_PreviewMeshRotationYaw)
	float PreviewMeshRotationYaw = 0.f;

	// 트랩 철거 및 재설치 용도 포커스 변수 
	UPROPERTY(Replicated)
	TObjectPtr<class ATrapBase> FocusTrap;

	TWeakObjectPtr<class ATrapperGameState> TrapperGameState;

	/// On_Rep -----------------------------------------------

	UFUNCTION()
	void OnRep_PreviewMesh();

	UFUNCTION()
	void OnRep_PreviewRange();

	UFUNCTION()
	void OnRep_PreviewCouple();

	UFUNCTION()
	void OnRep_PreviewNiagara();

	UFUNCTION()
	void OnRep_PreviewRangeDecal();

	UFUNCTION()
	void OnRep_TrapPartsTableIndex();

	UFUNCTION()
	void OnRep_PreviewMeshRotationYaw();

	/// RPC --------------------------------------------------------

	// 프리뷰 메쉬 생성 및 초기화
	UFUNCTION(Server, Reliable)
	void ServerRPCSetPreviewMesh(uint8 TableIndex);

	// 프리뷰 메쉬 트랜스폼 업데이트
	UFUNCTION(Server, Unreliable)
	void ServerRPCUpdatePreviewMeshTransform(FTransform PMTransform, FTransform PCTransform, FTransform PRTransform, float PMRotationYaw);

	// 프리뷰 메쉬 머테리얼 업데이트
	UFUNCTION(Server, Unreliable)
	void ServerRPCUpdatePreviewMeshMaterial(bool bMaterialBlue, class ATrapSnapTrigger* TSTrigger);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCUpdatePreviewMeshMaterial(bool bMaterialBlue, class ATrapSnapTrigger* TSTrigger);

	//프리뷰 메쉬 해제 및 제거
	UFUNCTION(Server, Reliable)
	void ServerRPCRealesePreviewMesh(class ATrapSnapTrigger* TSTrigger);

	// 프리뷰 메쉬 설치
	UFUNCTION(Server, Reliable)
	void ServerRPCPlaceTrap(class ATrapSnabEnviromentBase* ClientTrapSnabEnviromentActor, class ATrapSnapTrigger* ClientTrapSnabTrigger);

	//// 프리뷰 메쉬 교체 (메쉬 인덱스 변경)
	//UFUNCTION(Server, Reliable)
	//void ServerRPCChangePreviewMesh();

	UFUNCTION(Server, Reliable)
	void ServerRPCSetHiddenNiagara(bool Value);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSetHiddenNiagara(bool Value);

	// 기요틴
	UFUNCTION(Server, Reliable)
	void ServerRPCSnapGuillotineControl(ATrapBase* SnapGuillotine, bool bIsHidden, bool bMaterialBlue);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSnapGuillotineControl(ATrapBase* SnapGuillotine, bool bIsHidden, bool bMaterialBlue);


	UFUNCTION(Server, Reliable)
	void ServerRPCSetHiddenPreviewMesh(bool bIsHidden);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSetHiddenPreviewMesh(bool bIsHidden);

	// 트랩 철거
	UFUNCTION(Server, Reliable)
	void ServerRPCDestroyFocusTrap(ATrapBase* Trap);

	// 트랩 재설치
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCReInstallTrap();

	UFUNCTION(Server, Reliable)
	void ServerRPCReInstallTrap(ATrapBase* Trap);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSnapTriggerSetTrapNullptr(ATrapSnapTrigger* TSTrigger, ATrapBase* Trap);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCGuillotineSpawn(ATrapSnapTrigger* TSTrigger);

	// 트랩 설치 애니메이션
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCStartInstallAnim();

	UFUNCTION(Server, Reliable)
	void ServerRPCStartInstallAnim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCEndInstallAnim();

	UFUNCTION(Server, Reliable)
	void ServerRPCEndInstallAnim();

	/*UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCPlaySoundInstallTrap();*/

	UFUNCTION(Client, Unreliable)
	void ClientRPCPlaySoundInstallTrap();

	void SafeReleasePreviewMesh();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSafeReleasePreviewMesh();

	/*UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCFirstCoupleInstall(ADecalActor* PRDecal);*/

	UFUNCTION(Server, Reliable)
	void ServerRPCFirstCoupleInstall();

	//UFUNCTION(NetMulticast, Reliable)
	//void MulticastRPCSnapGuillotineMaterial

	UFUNCTION(Server, Reliable)
	void ServerRPCUpdateScoreHUD();

	// 임시 치트
	//void ShowMeTheMoney(); // 키보드 1
	void ShowScoreHUD(); // 키보드 2
	void UpdateScoreHUD(); // 키보드 3
	void SpawnMonsterSpikeTraps(); // 키보드 4

	UFUNCTION(Server, Reliable)
	void ServerRPCSpawnMonsterSpikeTraps(FTransform Transform);
};
