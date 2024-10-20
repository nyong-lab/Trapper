// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameResultHUD.generated.h"

/**
 * 
 */
UCLASS()
class TRAPPERPROJECT_API UGameResultHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void BackTitle();

public:
	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<class UTextBlock> MainText;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<class UTextBlock> DescriptionText;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<class UTextBlock> ToBeContinuedText;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<class UButton> BackTitleButton;

	void SetGameEndText(bool IsGameClear);
};
