// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Trap/Utillity/TrapEnums.h"
#include "TrapInstallLocationParts.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FTrapInstallLocationParts : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrapInstallLocation LocationType = ETrapInstallLocation::E_Floor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* Image = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = "";
};