// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/TrapSelectHUD.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/TextBlock.h"

#include "Trap/Utillity/TrapHUDParts.h"
#include "Trap/Utillity/TrapActivationMethodParts.h"
#include "Trap/Utillity/TrapCCParts.h"
#include "Trap/Utillity/TrapInstallLocationParts.h"

UTrapSelectHUD::UTrapSelectHUD(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	LoadTable();
	ConvertToMap();
}

void UTrapSelectHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	InstallDisabledText->SetVisibility(ESlateVisibility::Collapsed);
}

void UTrapSelectHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

/// -----------------------------------------------------------------------------------
///										Hovered
/// -----------------------------------------------------------------------------------

void UTrapSelectHUD::OnHovered(ETrapType type, ETrapHUDType bEnabled)
{
	CurrentType = type;

	float Alpha1 = 0.4f;
	float Alpha2 = 0.6f;
	float Alpha3 = 0.8f;
	FLinearColor EnabledColor = FLinearColor(1, 0.86, 0.44, 1);

	SpikePanel->SetRenderScale(FVector2D(0.65, 0.65));
	Bear->SetRenderScale(FVector2D(0.65, 0.65));
	OilBag->SetRenderScale(FVector2D(0.65, 0.65));
	ClapPanel->SetRenderScale(FVector2D(0.65, 0.65));
	Plank->SetRenderScale(FVector2D(0.65, 0.65));
	BarrelPanel->SetRenderScale(FVector2D(0.65, 0.65));
	Guillotine->SetRenderScale(FVector2D(0.65, 0.65));
	Spear->SetRenderScale(FVector2D(0.65, 0.65));
	
	Spike->SetColorAndOpacity(FLinearColor(1, 1, 1, Alpha1));
	Bear->SetColorAndOpacity(FLinearColor(1, 1, 1, Alpha1));
	OilBag->SetColorAndOpacity(FLinearColor(1, 1, 1, Alpha1));
	Clap->SetColorAndOpacity(FLinearColor(1, 1, 1, Alpha1));
	Plank->SetColorAndOpacity(FLinearColor(1, 1, 1, Alpha1));
	Barrel->SetColorAndOpacity(FLinearColor(1, 1, 1, Alpha1));
	Guillotine->SetColorAndOpacity(FLinearColor(1, 1, 1, Alpha1));
	Spear->SetColorAndOpacity(FLinearColor(1, 1, 1, Alpha1));

	SpikeText->SetColorAndOpacity(FLinearColor(1, 1, 1, Alpha1));
	ClapText->SetColorAndOpacity(FLinearColor(1, 1, 1, Alpha1));
	BarrelText->SetColorAndOpacity(FLinearColor(1, 1, 1, Alpha1));

	switch(type)
	{
	case ETrapType::Spike:
		Spear->SetOpacity(Alpha3);
		Guillotine->SetOpacity(Alpha2);
		Bear->SetOpacity(Alpha3);
		OilBag->SetOpacity(Alpha2);
		TargetAngle = -135;
		if(bEnabled == ETrapHUDType::E_Disabled)
		{
			Spike->SetColorAndOpacity(FLinearColor(1,0.1458, 0.1458, 1));
			SpikeText->SetColorAndOpacity(FLinearColor(1, 0.1458, 0.1458, 1));
		}
		else
		{
			Spike->SetColorAndOpacity(EnabledColor);
			SpikeText->SetColorAndOpacity(FColor::White);
		}
		break;

	case ETrapType::Bear:
		Spike->SetOpacity(Alpha3);
		SpikeText->SetOpacity(Alpha3);
		Spear->SetOpacity(Alpha2);
		OilBag->SetOpacity(Alpha3);
		Clap->SetOpacity(Alpha2);
		ClapText->SetOpacity(Alpha2);
		TargetAngle = -180;
		if(bEnabled == ETrapHUDType::E_Disabled)
		{
			Bear->SetColorAndOpacity(FLinearColor(1, 0.1458, 0.1458, 1));
		}
		else
		{
			Bear->SetColorAndOpacity(EnabledColor);
		}
		break;

	case ETrapType::Plank:
		Barrel->SetOpacity(Alpha3);
		BarrelText->SetOpacity(Alpha3);
		Guillotine->SetOpacity(Alpha2);
		Plank->SetOpacity(Alpha3);
		Clap->SetOpacity(Alpha2);
		ClapText->SetOpacity(Alpha2);
		TargetAngle = 45;
		if(bEnabled == ETrapHUDType::E_Disabled)
		{
			Plank->SetColorAndOpacity(FLinearColor(1, 0.1458, 0.1458, 1));
		}
		else
		{
			Plank->SetColorAndOpacity(EnabledColor);
		}
		break;

	case ETrapType::GuillotinePendulum:
		Spear->SetOpacity(Alpha3);
		Spike->SetOpacity(Alpha2);
		SpikeText->SetOpacity(Alpha2);
		Barrel->SetOpacity(Alpha3);
		BarrelText->SetOpacity(Alpha3);
		Plank->SetOpacity(Alpha2);
		TargetAngle = -45;
		if(bEnabled == ETrapHUDType::E_Disabled)
		{
			Guillotine->SetColorAndOpacity(FLinearColor(1, 0.1458, 0.1458, 1));
		}
		else
		{
			Guillotine->SetColorAndOpacity(EnabledColor);
		}
		break;

	case ETrapType::GunpowderBarrel:
		Guillotine->SetOpacity(Alpha3);
		Plank->SetOpacity(Alpha3);
		Spear->SetOpacity(Alpha2);
		Clap->SetOpacity(Alpha2);
		ClapText->SetOpacity(Alpha2);
		TargetAngle = 0;
		if(bEnabled == ETrapHUDType::E_Disabled)
		{
			Barrel->SetColorAndOpacity(FLinearColor(1, 0.1458, 0.1458, 1));
			BarrelText->SetColorAndOpacity(FLinearColor(1, 0.1458, 0.1458, 1));
		}
		else
		{
			Barrel->SetColorAndOpacity(EnabledColor);
			BarrelText->SetColorAndOpacity(FColor::White);
		}
		break;

	case ETrapType::OilBag:
		Bear->SetOpacity(Alpha3);
		Clap->SetOpacity(Alpha3);
		ClapText->SetOpacity(Alpha3);
		Plank->SetOpacity(Alpha2);
		Spike->SetOpacity(Alpha2);
		SpikeText->SetOpacity(Alpha2);
		TargetAngle = 135;
		if(bEnabled == ETrapHUDType::E_Disabled)
		{
			OilBag->SetColorAndOpacity(FLinearColor(1, 0.1458, 0.1458, 1));
		}
		else
		{
			OilBag->SetColorAndOpacity(EnabledColor);
		}
		break;

	case ETrapType::Clap:
		OilBag->SetOpacity(Alpha3);
		Bear->SetOpacity(Alpha2);
		Plank->SetOpacity(Alpha3);
		Barrel->SetOpacity(Alpha2);
		BarrelText->SetOpacity(Alpha2);
		TargetAngle = 90;
		if(bEnabled == ETrapHUDType::E_Disabled)
		{
			Clap->SetColorAndOpacity(FLinearColor(1, 0.1458, 0.1458, 1));
			ClapText->SetColorAndOpacity(FLinearColor(1, 0.1458, 0.1458, 1));
		}
		else
		{
			Clap->SetColorAndOpacity(EnabledColor);
			ClapText->SetColorAndOpacity(FColor::White);
		}
		break;

	case ETrapType::Spear:
		Spike->SetOpacity(Alpha3);
		SpikeText->SetOpacity(Alpha3);
		Bear->SetOpacity(Alpha2);
		Guillotine->SetOpacity(Alpha3);
		Barrel->SetOpacity(Alpha2);
		BarrelText->SetOpacity(Alpha2);
		TargetAngle = -90;
		if(bEnabled == ETrapHUDType::E_Disabled)
		{
			Spear->SetColorAndOpacity(FLinearColor(1, 0.1458, 0.1458, 1));
		}
		else
		{
			Spear->SetColorAndOpacity(EnabledColor);
		}
		break;

	case ETrapType::None:
		break;
	}

	if(type != ETrapType::None)
	{
		CenterPanel->SetVisibility(ESlateVisibility::Visible);
		CenterRing->SetVisibility(ESlateVisibility::Visible);

		TrapName->SetText(FText::FromString(TrapData[type].Name));
		Cost->SetText(FText::FromString(TrapCosts[type]));
		Damage->SetText(FText::FromString(TrapDamages[type]));

		InstallPosImage->SetBrushFromTexture(LocationData[TrapData[type].LocationType].Image);
		InstallPosText->SetText(FText::FromString(LocationData[TrapData[type].LocationType].Description));

		ActivateTypeImage->SetBrushFromTexture(ActivationData[TrapData[type].ActivationType].Image);
		ActivateTypeText->SetText(FText::FromString(ActivationData[TrapData[type].ActivationType].Description));

		if(TrapData[type].CCType == ETrapCCType::E_None)
		{
			CCTypeImage->SetVisibility(ESlateVisibility::Collapsed);
			CCTypeText->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			CCTypeImage->SetBrushFromTexture(CCData[TrapData[type].CCType].Image);
			CCTypeText->SetText(FText::FromString(CCData[TrapData[type].CCType].Description));
		
			CCTypeImage->SetVisibility(ESlateVisibility::Visible);
			CCTypeText->SetVisibility(ESlateVisibility::Visible);
		}

		if(bEnabled == ETrapHUDType::E_Enabled)
		{
			Cost->SetColorAndOpacity(FColor::White);
		}
		else
		{
			Cost->SetColorAndOpacity(FColor::Red);
		}
	}
	else
	{
		CenterPanel->SetVisibility(ESlateVisibility::Collapsed);
		CenterRing->SetVisibility(ESlateVisibility::Collapsed);
	}
}


void UTrapSelectHUD::LerpAngle(float DeltaTime)
{
	if(CurrentType != ETrapType::None)
	{
		float CurrentAngle = SideRing->GetRenderTransformAngle();
		float Delta = FMath::Fmod(TargetAngle - CurrentAngle + 360.0f, 360.0f);
		if(Delta > 180.0f)	Delta -= 360.0f;
		float Result = CurrentAngle + Delta * DeltaTime * 7;
		SideRing->SetRenderTransformAngle(Result);
		CenterRing->SetRenderTransformAngle(Result);

		float CurrnetScale = 1.f;
		float TargetScale = 1.f;
		float ScaleValue = 10;

		switch(CurrentType)
		{
		case ETrapType::Spike:
			CurrnetScale = SpikePanel->GetRenderTransform().Scale.X;
			Result = FMath::Lerp(CurrnetScale, TargetScale, DeltaTime * ScaleValue);
			SpikePanel->SetRenderScale(FVector2D(Result, Result));
			break;
		
		case ETrapType::Bear:
			CurrnetScale = Bear->GetRenderTransform().Scale.X;
			Result = FMath::Lerp(CurrnetScale, TargetScale, DeltaTime * ScaleValue);
			Bear->SetRenderScale(FVector2D(Result, Result));
			break;
		
		case ETrapType::Plank:
			CurrnetScale = Plank->GetRenderTransform().Scale.X;
			Result = FMath::Lerp(CurrnetScale, TargetScale, DeltaTime * ScaleValue);
			Plank->SetRenderScale(FVector2D(Result, Result));
			break;
		
		case ETrapType::GuillotinePendulum:
			CurrnetScale = Guillotine->GetRenderTransform().Scale.X;
			Result = FMath::Lerp(CurrnetScale, TargetScale, DeltaTime * ScaleValue);
			Guillotine->SetRenderScale(FVector2D(Result, Result));
			break;
		
		case ETrapType::GunpowderBarrel:
			CurrnetScale = BarrelPanel->GetRenderTransform().Scale.X;
			Result = FMath::Lerp(CurrnetScale, TargetScale, DeltaTime * ScaleValue);
			BarrelPanel->SetRenderScale(FVector2D(Result, Result));
			break;
		
		case ETrapType::OilBag:
			CurrnetScale = OilBag->GetRenderTransform().Scale.X;
			Result = FMath::Lerp(CurrnetScale, TargetScale, DeltaTime * ScaleValue);
			OilBag->SetRenderScale(FVector2D(Result, Result));
			break;
		
		case ETrapType::Clap:
			CurrnetScale = ClapPanel->GetRenderTransform().Scale.X;
			Result = FMath::Lerp(CurrnetScale, TargetScale, DeltaTime * ScaleValue);
			ClapPanel->SetRenderScale(FVector2D(Result, Result));
			break;
		
		case ETrapType::Spear:
			CurrnetScale = Spear->GetRenderTransform().Scale.X;
			Result = FMath::Lerp(CurrnetScale, TargetScale, DeltaTime * ScaleValue);
			Spear->SetRenderScale(FVector2D(Result, Result));
			break;
		}
	}
}

/// -----------------------------------------------------------------------------------
///									Getter, Setter
/// -----------------------------------------------------------------------------------


FVector2D UTrapSelectHUD::GetSize()
{
	return CenterRing->GetCachedGeometry().GetDrawSize();
}

FVector2D UTrapSelectHUD::GetCenterPos()
{
	FGeometry Geometry = CenterRing->GetCachedGeometry();
	FVector2D pixelPos, viewportPos;
	USlateBlueprintLibrary::LocalToViewport(GetWorld(), Geometry, FVector2D(0, 0), pixelPos, viewportPos);

	viewportPos *= UWidgetLayoutLibrary::GetViewportScale(GetWorld());
	viewportPos += GetSize() * 0.5f;

	//UE_LOG(LogTemp, Warning, TEXT("CenterPos : %s"), *viewportPos.ToString());
	return viewportPos;
}

void UTrapSelectHUD::UpgradeCost(float ratio)
{
	for(const TPair<ETrapType, FTrapHUDParts>& Pair : TrapData)
	{
		FString OriginCost = Pair.Value.Cost;
		int32 Integer = FCString::Atoi(*OriginCost);
		Integer *= ratio;
		TrapCosts[Pair.Key] = FString::FromInt(Integer);
	}
}

void UTrapSelectHUD::UpgradeDamage(float ratio)
{
	for(const TPair<ETrapType, FTrapHUDParts>& Pair : TrapData)
	{
		FString OriginDmg = Pair.Value.Damage;
		int32 Integer = FCString::Atoi(*OriginDmg);
		Integer *= ratio;
		TrapDamages[Pair.Key] = FString::FromInt(Integer);
	}
}

/// -----------------------------------------------------------------------------------
///									테이블 로드
/// -----------------------------------------------------------------------------------


void UTrapSelectHUD::LoadTable()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableFinder(
		TEXT("/Script/Engine.DataTable'/Game/Blueprints/Data/DT_TrapHUD.DT_TrapHUD'"));

	if(DataTableFinder.Succeeded())
	{
		TrapTable = DataTableFinder.Object;
	}

	//
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableFinder2(
		TEXT("/Script/Engine.DataTable'/Game/Blueprints/Widget/TrapHUD/DT_TrapInstallationLocation.DT_TrapInstallationLocation'"));

	if(DataTableFinder2.Succeeded())
	{
		LocationTable = DataTableFinder2.Object;
	}

	//
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableFinder3(
		TEXT("/Script/Engine.DataTable'/Game/Blueprints/Widget/TrapHUD/DT_TrapActivationMethod.DT_TrapActivationMethod'"));

	if(DataTableFinder3.Succeeded())
	{
		ActiveTable = DataTableFinder3.Object;
	}

	//
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableFinder4(
		TEXT("/Script/Engine.DataTable'/Game/Blueprints/Widget/TrapHUD/DT_TrapCC.DT_TrapCC'"));

	if(DataTableFinder4.Succeeded())
	{
		CCTable = DataTableFinder4.Object;
	}
}

void UTrapSelectHUD::ConvertToMap()
{
	TArray<FName> RowNames;
	RowNames = TrapTable->GetRowNames();
	for(const FName& RowName : RowNames)
	{
		FTrapHUDParts* Data = TrapTable->FindRow<FTrapHUDParts>(RowName, FString());
		TrapData.Add(Data->Type, *Data);
		TrapCosts.Add(Data->Type, Data->Cost);
		TrapDamages.Add(Data->Type, Data->Damage);
	}


	RowNames = LocationTable->GetRowNames();
	for(const FName& RowName : RowNames)
	{
		FTrapInstallLocationParts* Data = LocationTable->FindRow<FTrapInstallLocationParts>(RowName, FString());
		LocationData.Add(Data->LocationType, *Data);
	}


	RowNames = ActiveTable->GetRowNames();
	for(const FName& RowName : RowNames)
	{
		FTrapActivationMethodParts* Data = ActiveTable->FindRow<FTrapActivationMethodParts>(RowName, FString());
		ActivationData.Add(Data->ActivationType, *Data);
	}


	RowNames = CCTable->GetRowNames();
	for(const FName& RowName : RowNames)
	{
		FTrapCCParts* Data = CCTable->FindRow<FTrapCCParts>(RowName, FString());
		CCData.Add(Data->CCType, *Data);
	}
}
