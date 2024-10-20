// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPC.generated.h"

/**
 * 
 */
UCLASS()
class TRAPPERPROJECT_API AMainPC : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSubclassOf<class UMainWidgetBase> MainWidgetClass;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "UI")
	class UMainWidgetBase* MainWidgetObject;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSubclassOf<class UJoinWidgetBase> JoinWidgetClass;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "UI")
	class UJoinWidgetBase* JoinWidgetObject;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSubclassOf<class ULoginWidgetBase> LoginWidgetClass;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "UI")
	class ULoginWidgetBase* LoginWidgetObject;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSubclassOf<class UAfterJoinWidgetBase> AfterJoinWidgetClass;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "UI")
	class UAfterJoinWidgetBase* AfterJoinWidgetObject;

	UFUNCTION(BlueprintCallable)
	void SendLoginPacket(FString ID, FString Password);

	UFUNCTION(BlueprintCallable)
	void RecvLoginPacket();

	void ShowAfterJoinWidget(bool Show);
	void ShowJoinWidget(bool Show);
	void ShowLoginWidget(bool Show);
	void ShowMainWidget(bool Show);
	void ShowDuplicateIdWidget(bool Duplicated);


};
