// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MiniMapHUD.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"

#include "Kismet/GameplayStatics.h"
#include "Character/TrapperPlayer.h"
#include "Camera/CameraComponent.h"

#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"


void UMiniMapHUD::NativeConstruct()
{
	Super::NativeConstruct();

	GetWorld()->GetTimerManager().SetTimer(
		FindPlayerTimer, this, &UMiniMapHUD::FindPlayer, 0.1, true);

	RemotePlayerSlot = Cast<UCanvasPanelSlot>(RemotePlayerIcon->Slot);

	// 머테리얼 파라미터 컬렉션을 로드
	if(MaterialParameterCollection)
	{
		CollectionInstance = GetWorld()->GetParameterCollectionInstance(MaterialParameterCollection);

		CollectionInstance->GetScalarParameterValue(FName("Zoom"), Zoom);
		CollectionInstance->GetScalarParameterValue(FName("TopBorder"), TopBorder);
		CollectionInstance->GetScalarParameterValue(FName("BottomBorder"), BottomBorder);
		CollectionInstance->GetScalarParameterValue(FName("LeftBorder"), LeftBorder);
		CollectionInstance->GetScalarParameterValue(FName("RightBorder"), RightBorder);

		Height = BottomBorder - TopBorder;
		Width = RightBorder - LeftBorder;
	}
}


void UMiniMapHUD::NativeDestruct()
{
	Super::NativeDestruct();
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void UMiniMapHUD::UpdateMinimap(FVector ActorLocation)
{
	if(IsValid(Player) && CollectionInstance)
	{
		float Angle = Player->Camera->GetComponentRotation().Yaw;
		PlayerSight->SetRenderTransformAngle(Angle);
		Angle = Player->GetActorRotation().Yaw;
		PlayerIcon->SetRenderTransformAngle(Angle);

		CollectionInstance->SetScalarParameterValue(FName("PlayerX"), ActorLocation.X);
		CollectionInstance->SetScalarParameterValue(FName("PlayerY"), ActorLocation.Y);

		PlayerLocation = FVector2D(ActorLocation.X, ActorLocation.Y);
	}

	if(IsValid(TeamPlayer) && CollectionInstance)
	{
		float Angle = TeamPlayer->GetActorRotation().Yaw;
		RemotePlayerIcon->SetRenderTransformAngle(Angle);
		RemotePlayerSlot->SetPosition(ConvertToMapCoord(TeamPlayer->GetActorLocation()));
	}
}


FVector2D UMiniMapHUD::ConvertToMapCoord(FVector Location)
{
	FVector2D ImgSize = MapImage->GetCachedGeometry().GetLocalSize();

	float rLeft = PlayerLocation.X - Width * Zoom * 0.5f;
	float rRight = PlayerLocation.X + Width * Zoom * 0.5f;
	float rTop = PlayerLocation.Y - Height * Zoom * 0.5f;
	float rBottom = PlayerLocation.Y + Height * Zoom * 0.5f;

	float NormalizeX = (Location.X - rLeft) / (rRight - rLeft);
	float NormalizeY = (Location.Y - rTop) / (rBottom - rTop);
	
	return FVector2D(NormalizeY * ImgSize.Y, (1 - NormalizeX) * ImgSize.X);
}


void UMiniMapHUD::FindPlayer()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATrapperPlayer::StaticClass(), OutActors);

	for(const auto& Pawn : OutActors)
	{
		ATrapperPlayer* TrapperPlayer = Cast<ATrapperPlayer>(Pawn);
		if(!TrapperPlayer) continue;

		if(TrapperPlayer->IsLocallyControlled())
		{
			Player = TrapperPlayer;
		}
		else
		{
			TeamPlayer = TrapperPlayer;
		}
	}

	if(IsValid(Player) && IsValid(TeamPlayer))
	{
		GetWorld()->GetTimerManager().ClearTimer(FindPlayerTimer);
	}
}
