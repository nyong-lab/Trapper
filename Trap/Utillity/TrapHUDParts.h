// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Trap/Utillity/TrapEnums.h"
#include "TrapHUDParts.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FTrapHUDParts : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrapType Type = ETrapType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrapInstallLocation LocationType = ETrapInstallLocation::E_Floor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrapActivationMethod ActivationType = ETrapActivationMethod::E_DefaultArrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrapCCType CCType = ETrapCCType::E_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Cost = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Damage = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* PriviewImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = "";
};