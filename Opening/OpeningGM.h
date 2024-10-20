// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OpeningGM.generated.h"

/**
 * 
 */
UCLASS()
class TRAPPERPROJECT_API AOpeningGM : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Server, Reliable) // 서버에서 호출되는 함수
	void ServerTravelToTrapper(); // Trapper 레벨로 이동
};
