// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogDataTables.generated.h"

UENUM()
enum class ECharacterName : uint8
{
	Diana			UMETA(DisplayName = "Diana"),
	Riana			UMETA(DisplayName = "Riana"),
	Death			UMETA(DisplayName = "Death"),
	Humanity		UMETA(DisplayName = "Humanity"),
};

USTRUCT()
struct FDialogInfo : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	FDialogInfo()
	: Character(ECharacterName::Riana)
	, Dialog()
	, Time(3.f)
	, bIsEnd(false)
	, EventCode()
	, VoiceFile()
	{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite) ECharacterName Character;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Dialog;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Time;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) uint8 bIsEnd : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<int32> EventCode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundWave* VoiceFile;

};

UCLASS()
class TRAPPERPROJECT_API ADialogDataTables : public AActor
{
	GENERATED_BODY()
};
