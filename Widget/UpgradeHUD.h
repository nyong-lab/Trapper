// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeHUD.generated.h"

/**
 * 
 */
UCLASS()
class TRAPPERPROJECT_API UUpgradeHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	void OnHovered(uint8 parts);
	void OnUnhovered(uint8 parts);
	void SetLastTime(int32 time);
	void SetPillarTextVisibility(bool Arg);
	void SetTutorialBackgroundVisibility(bool Arg);

public:
	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UUpgradePartsHUD> Parts1;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UUpgradePartsHUD> Parts2;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UUpgradePartsHUD> Parts3;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UUpgradePartsHUD> Parts4;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UImage> TutorialBackground;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UTextBlock> LastTime;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UTextBlock> PillarText;
};
