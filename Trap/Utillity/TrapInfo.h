// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Trap/Utillity/TrapEnums.h"
#include "TrapInfo.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FTrapInfo : public FTableRowBase
{
	GENERATED_BODY()

public:

	// 설치 비용
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InstallCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IncreaseDamage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IncreaseSpeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Strength = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IncreaseStrength = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0;
};