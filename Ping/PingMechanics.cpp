﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Ping/PingMechanics.h"

#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"

#include "Character/TrapperPlayer.h"
#include "PlayerPing.h"
#include "MarkerComponent/MapMarkerComponent.h"
#include "MarkerComponent/MapMarkerType.h"
#include "Widget/PingSelectHUD.h"
#include "Framework/TrapperPlayerController.h"
#include "Map/MapMechanics.h"
#include "Upgrade/UpgradeMechanics.h"


UPingMechanics::UPingMechanics()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UPingMechanics::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = Cast<ATrapperPlayer>(GetOwner());
	PlayerController = Cast<ATrapperPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	
	SetupInputComponent();
}


void UPingMechanics::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}


/// -----------------------------------------------------------------------------------------------------------
///                                            인풋
/// -----------------------------------------------------------------------------------------------------------

void UPingMechanics::SetupInputComponent()
{
	if(!IsValid(PlayerController))
		return;

	UEnhancedInputComponent* UIC = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if(IsValid(UIC))
	{
		UIC->BindAction(EnterAction, ETriggerEvent::Started, this, &UPingMechanics::EnterPingMode);
		UIC->BindAction(EnterAction, ETriggerEvent::Completed, this, &UPingMechanics::ExitPingMode);
		UIC->BindAction(CancelAction, ETriggerEvent::Triggered, this, &UPingMechanics::Cancel);
	}
}


void UPingMechanics::EnterPingMode()
{
	if( !IsValid(PlayerRef)
		|| !PlayerRef->IsLocallyControlled()
		|| PlayerRef->IsTrapSelectMode()
		|| PlayerRef->MapMechanics->IsOpenedWorldMap()
		|| PlayerRef->UpgradeMechanics->IsOnUpgradeMode()
		|| bIsBlocked)
	{
		return;
	}

	bIsPingSelectModeActivated = true;

	// 커서 관련
	PlayerRef->ShowCursor();
	PlayerRef->SetCursorToCenter();

	// HUD
	if(!IsValid(SelectHUDRef))
	{
		SelectHUDRef = PlayerController->PingSelectHUDRef;
	}

	if(IsValid(SelectHUDRef))
	{
		SelectHUDRef->SetVisibility(ESlateVisibility::Visible);
		GetWorld()->GetTimerManager().SetTimer(
			UpdateHoverTimer, this, &UPingMechanics::CheckOnHovered, 0.03, true);
	}
}


void UPingMechanics::ExitPingMode()
{
	if(!IsValid(PlayerRef)
		|| !PlayerRef->IsLocallyControlled()
		|| !bIsPingSelectModeActivated )
	{
		return;
	}
	
	bIsPingSelectModeActivated = false;

	PlayerRef->HideCursor();
	GetWorld()->GetTimerManager().ClearTimer(UpdateHoverTimer);
	if(IsValid(SelectHUDRef))
	{
		SelectHUDRef->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 스폰 -----------------------------------------------------------------------------
	FVector Location = CalculateSpawnDirection();
	SpawnPing(HoveredPingType, Location);
}


void UPingMechanics::Cancel()
{
	if(!PlayerRef->IsLocallyControlled() 
		|| !bIsPingSelectModeActivated)	
	{
		return;
	}

	bIsPingSelectModeActivated = false;
	PlayerRef->HideCursor();
	GetWorld()->GetTimerManager().ClearTimer(UpdateHoverTimer);
	
	if(IsValid(SelectHUDRef))
	{
		SelectHUDRef->SetVisibility(ESlateVisibility::Collapsed);
	}
}


void UPingMechanics::SpawnPing(ESelectPingType PingType, FVector Location)
{
	switch(PingType)
	{
	case ESelectPingType::E_Default:
		if(PlayerRef->HasAuthority())
			MulticastRPCSpawnPlayerPing(EPlayerPingType::E_Default1P, Location);
		else	ServerRPCSpawnPlayerPing(EPlayerPingType::E_Default2P, Location);
		break;

	case ESelectPingType::E_TrapInstall:
		PlayerController->CreatePingInformationMessage(ESelectPingType::E_TrapInstall);
		if(PlayerRef->HasAuthority())
			MulticastRPCSpawnPlayerPing(EPlayerPingType::E_TrapInstall1P, Location);
		else	ServerRPCSpawnPlayerPing(EPlayerPingType::E_TrapInstall2P, Location);
		break;

	case ESelectPingType::E_TrapActivate:
		PlayerController->CreatePingInformationMessage(ESelectPingType::E_TrapActivate);
		if(PlayerRef->HasAuthority())
			MulticastRPCSpawnPlayerPing(EPlayerPingType::E_TrapActivate1P, Location);
		else	ServerRPCSpawnPlayerPing(EPlayerPingType::E_TrapActivate2P, Location);
		break;

	case ESelectPingType::E_Warning:
		PlayerController->CreatePingInformationMessage(ESelectPingType::E_Warning);
		if(PlayerRef->HasAuthority())
			MulticastRPCSpawnPlayerPing(EPlayerPingType::E_Warning1P, Location);
		else	ServerRPCSpawnPlayerPing(EPlayerPingType::E_Warning2P, Location);
		break;

	case ESelectPingType::E_Assemble:
		PlayerController->CreatePingInformationMessage(ESelectPingType::E_Assemble);
		if(PlayerRef->HasAuthority())
			MulticastRPCSpawnPlayerPing(EPlayerPingType::E_Assemble1P, Location);
		else	ServerRPCSpawnPlayerPing(EPlayerPingType::E_Assemble2P, Location);
		break;

	case ESelectPingType::None:
		break;
	}

	if(PingType != ESelectPingType::None
		&& IsValid(PlaceSound) && IsValid(PlayerRef->AudioComponent))
	{
		PlayerRef->AudioComponent->SetSound(PlaceSound);
		PlayerRef->AudioComponent->Play();
	}
}

FVector UPingMechanics::CalculateSpawnDirection()
{
	// 라인트레이스로 방향 계산
	UCameraComponent* Camera = PlayerRef->GetComponentByClass<UCameraComponent>();
	if(!IsValid(Camera))
		return PlayerRef->GetActorLocation();

	FHitResult Hit;
	FVector Start = Camera->GetComponentLocation();
	FVector End = Camera->GetComponentLocation() + Camera->GetForwardVector() * 10000.f;
	ECollisionChannel TraceChannel = ECollisionChannel::ECC_Visibility;

	bool bIsCatched = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel);
	if(bIsCatched)
	{
		return Hit.ImpactPoint;
	}
	else
	{
		return PlayerRef->GetActorLocation();
	}
}


void UPingMechanics::CheckOnHovered()
{
	if(!GEngine || !GEngine->GameViewport || !IsValid(SelectHUDRef))
		return;

	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	FVector2D CenterOfViewport = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

	FVector2D MousePosition;
	GEngine->GameViewport->GetMousePosition(MousePosition);

	float Distance = FVector2D::Distance(CenterOfViewport, MousePosition);
	float Size = SelectHUDRef->GetSize().X;
	float Radius = Size * 0.2;	

	ESelectPingType CurrentHoveredPingType = ESelectPingType::None;
	if( Distance < Radius )
	{
		CurrentHoveredPingType = ESelectPingType::E_Default;
	}
	else
	{
		// 마우스 각도 계산
		FVector2D Normal = MousePosition - CenterOfViewport;
		Normal.Normalize();
		
		float Radians = FMath::Atan2(Normal.Y, Normal.X);
		float Degrees = FMath::RadiansToDegrees(Radians); //-180 ~ 180
		Degrees += 180;

		if( 45 < Degrees && Degrees <= 135)
			CurrentHoveredPingType = ESelectPingType::E_TrapInstall;
		else if( 135 < Degrees && Degrees <= 225)
			CurrentHoveredPingType = ESelectPingType::E_Assemble;
		else if ( 225 < Degrees && Degrees <= 315)
			CurrentHoveredPingType = ESelectPingType::E_Warning;
		else
			CurrentHoveredPingType = ESelectPingType::E_TrapActivate;
	}

	// 바뀌었을 때 HUD 업데이트
	if(CurrentHoveredPingType != HoveredPingType)
	{
		SelectHUDRef->OnHovered(CurrentHoveredPingType);
		SelectHUDRef->OnUnhovered(HoveredPingType);
	}

	HoveredPingType = CurrentHoveredPingType;
}


void UPingMechanics::DestroyPing()
{
	if(IsValid(OwnedPing))
	{
		GetWorld()->GetTimerManager().ClearTimer(DestroyPingTimer);
		OwnedPing->Destroy();
		OwnedPing = nullptr;
	}
}


void UPingMechanics::BlockPingMode()
{
	bIsBlocked = true;
	Cancel();
}


void UPingMechanics::UnblockPingMode()
{
	bIsBlocked = false;
}


/// --------------------------------------------------------------------------------------
///                                     RPC
/// --------------------------------------------------------------------------------------

void UPingMechanics::ServerRPCSpawnPlayerPing_Implementation(EPlayerPingType type, FVector Location)
{
	MulticastRPCSpawnPlayerPing(type, Location);
}

void UPingMechanics::MulticastRPCSpawnPlayerPing_Implementation(EPlayerPingType type, FVector Location)
{
	DestroyPing();

	FTransform SpawnTransform = FTransform(Location);
	APlayerPing* Ping = Cast<APlayerPing>(GetWorld()->SpawnActor<APlayerPing>(PingClass, SpawnTransform));
	Ping->SetOwner(PlayerRef);
	OwnedPing = Ping;

	switch(type)
	{
	case EPlayerPingType::E_Default1P:
		GetWorld()->GetTimerManager().SetTimer(
			DestroyPingTimer, this, &UPingMechanics::DestroyPing, DefulatLiftTime, false);
		Ping->SetType(EPlayerPingType::E_Default1P, true);
		break;

	case EPlayerPingType::E_Default2P:
		GetWorld()->GetTimerManager().SetTimer(
			DestroyPingTimer, this, &UPingMechanics::DestroyPing, DefulatLiftTime, false);
		Ping->SetType(EPlayerPingType::E_Default2P, false);
		break;

	case EPlayerPingType::E_TrapInstall1P:
		Ping->SetType(EPlayerPingType::E_TrapInstall1P, true);
		GetWorld()->GetTimerManager().SetTimer(
			DestroyPingTimer, this, &UPingMechanics::DestroyPing, InstallLifeTime, false);
		break;

	case EPlayerPingType::E_TrapInstall2P:
		Ping->SetType(EPlayerPingType::E_TrapInstall2P, false);
		GetWorld()->GetTimerManager().SetTimer(
			DestroyPingTimer, this, &UPingMechanics::DestroyPing, InstallLifeTime, false);
		break;

	case EPlayerPingType::E_TrapActivate1P:
		Ping->SetType(EPlayerPingType::E_TrapActivate1P, true);
		GetWorld()->GetTimerManager().SetTimer(
			DestroyPingTimer, this, &UPingMechanics::DestroyPing, ActivateLifeTime, false);

		break;

	case EPlayerPingType::E_TrapActivate2P:
		Ping->SetType(EPlayerPingType::E_TrapActivate2P, false);
		GetWorld()->GetTimerManager().SetTimer(
			DestroyPingTimer, this, &UPingMechanics::DestroyPing, ActivateLifeTime, false);
		break;

	case EPlayerPingType::E_Warning1P:
		Ping->SetType(EPlayerPingType::E_Warning1P, true);
		GetWorld()->GetTimerManager().SetTimer(
			DestroyPingTimer, this, &UPingMechanics::DestroyPing, AssembleLifeTime, false);
		break;

	case EPlayerPingType::E_Warning2P:
		Ping->SetType(EPlayerPingType::E_Warning2P, false);
		GetWorld()->GetTimerManager().SetTimer(
			DestroyPingTimer, this, &UPingMechanics::DestroyPing, AssembleLifeTime, false);
		break;

	case EPlayerPingType::E_Assemble1P:
		Ping->SetType(EPlayerPingType::E_Assemble1P, true);
		GetWorld()->GetTimerManager().SetTimer(
			DestroyPingTimer, this, &UPingMechanics::DestroyPing, WarningLifeTime, false);
		break;

	case EPlayerPingType::E_Assemble2P:
		Ping->SetType(EPlayerPingType::E_Assemble2P, false);
		GetWorld()->GetTimerManager().SetTimer(
			DestroyPingTimer, this, &UPingMechanics::DestroyPing, WarningLifeTime, false);
		break;
	}
}

