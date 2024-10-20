// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OpeningPC.generated.h"

/**
 * 
 */
UCLASS()
class TRAPPERPROJECT_API AOpeningPC : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UI")
	TSubclassOf<class UOpeningWidgetBase> OpeningWidgetClass;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "UI")
	class UOpeningWidgetBase* OpeningWidgetObject;


};
