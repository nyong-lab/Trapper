// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerPingType.h"
#include "PingMechanics.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TRAPPERPROJECT_API UPingMechanics : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPingMechanics();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;


/// ¸â¹ö ÇÔ¼ö ---------------------------------
public:
	bool IsPingSelectMode() const { return bIsPingSelectModeActivated; }
	void DestroyPing();
	void BlockPingMode();
	void UnblockPingMode();
	void SpawnPing(ESelectPingType PingType, FVector Location);
	void Cancel();

private:
	// ------------- Input -------------
	void SetupInputComponent();
	void EnterPingMode();
	void ExitPingMode();

	// ------------- °è»ê -------------
	FVector CalculateSpawnDirection();

	// ------------- Spawn RPC -------------
	UFUNCTION(Server, Reliable)
	void ServerRPCSpawnPlayerPing(EPlayerPingType type, FVector Location);
	void ServerRPCSpawnPlayerPing_Implementation(EPlayerPingType type, FVector Location);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCSpawnPlayerPing(EPlayerPingType type, FVector Location);
	void MulticastRPCSpawnPlayerPing_Implementation(EPlayerPingType type, FVector Location);

/// ¸â¹ö º¯¼ö ---------------------------------
public:
	// ------------- Input -------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	class UInputAction* EnterAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	class UInputAction* CancelAction;

	//  ------------- Sound  -------------
	UPROPERTY(EditDefaultsOnly, Category = "Marker")
	USoundBase* PlaceSound;

	// ------------- Class  -------------
	UPROPERTY(EditDefaultsOnly, Category = "Marker") 
	TSubclassOf<class APlayerPing> PingClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	TSubclassOf<class UPingSelectHUD> SelectHUDClass;

private:
	bool bIsPingSelectModeActivated = false;
	bool bIsBlocked = false;
	ESelectPingType HoveredPingType = ESelectPingType::None;

	UPROPERTY() class ATrapperPlayer* PlayerRef;
	UPROPERTY() class APlayerPing* OwnedPing;
	UPROPERTY() class ATrapperPlayerController* PlayerController;
	UPROPERTY() class UPingSelectHUD* SelectHUDRef;

	float DefulatLiftTime = 30.f;
	float InstallLifeTime = 10.f;
	float ActivateLifeTime = 5.f;
	float AssembleLifeTime = 10.f;
	float WarningLifeTime = 5.f;
	float CenterHUDRadius = 85.f;

	FTimerHandle DestroyPingTimer;
	FTimerHandle UpdateHoverTimer;
	void CheckOnHovered();
};
