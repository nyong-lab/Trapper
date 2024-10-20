// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Trap/Utillity/TrapEnums.h"
#include "TrapParts.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FTrapParts : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UStaticMesh> PreviewMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UStaticMesh> PreviewRange = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UStaticMesh> PreviewCouple = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ADecalActor> PreviewRangeDecal = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UParticleSystem> Effect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ATrapBase> ClassType = nullptr;

	// 설치 비용
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InstallCost = 0;

	// 트레이스 채널
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ETraceTypeQuery> TraceChannel = ETraceTypeQuery::TraceTypeQuery1;

	// 트랩 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrapType Type = ETrapType::None;

	// 일회성, 반복성 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bVolatile : 1 = false;

	// 스냅 함정인지 체크
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bCanSnap : 1 = false;

	// 천장 함정인지 체크
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsCeling : 1 = false;

	// 벽 함정인지 체크
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsWall : 1 = false;

	// 커플인지 체크
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsCouple : 1 = false;

	// 프리뷰 레인지가 있는지 체크
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bIsPreviewRange : 1 = false;
};