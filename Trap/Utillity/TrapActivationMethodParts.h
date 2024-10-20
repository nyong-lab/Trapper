// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Trap/Utillity/TrapEnums.h"
#include "TrapActivationMethodParts.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FTrapActivationMethodParts : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrapActivationMethod ActivationType = ETrapActivationMethod::E_DefaultArrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* Image = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = "";
};