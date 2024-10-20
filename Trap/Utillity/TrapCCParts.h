// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Trap/Utillity/TrapEnums.h"
#include "TrapCCParts.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FTrapCCParts : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrapCCType CCType = ETrapCCType::E_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* Image = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = "";
};