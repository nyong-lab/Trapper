// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class TRAPPERPROJECT_API ULoginWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* UserID;

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* Password;

	UPROPERTY(meta = (BindWidget))
	class UButton* ShowPasswordButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* LoginButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinButton;
	
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* WarningCanvasPanel;

	UFUNCTION()
	void OnShowPasswordButton();
	
	UFUNCTION()
	void OnJoinButton();

	UFUNCTION()
	void OnLoginButton();

	void InitializeUI();

	void ChangeLoginFailedUI();
};
