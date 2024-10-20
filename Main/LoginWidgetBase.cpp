// Fill out your copyright notice in the Description page of Project Settings.


#include "Main/LoginWidgetBase.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/EditableTextBox.h"
#include "MainPC.h"

void ULoginWidgetBase::NativeConstruct()
{
	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &ULoginWidgetBase::OnLoginButton);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ULoginWidgetBase::OnJoinButton);
	}
	
	if (ShowPasswordButton)
	{
		ShowPasswordButton->OnClicked.AddDynamic(this, &ULoginWidgetBase::OnShowPasswordButton);
	}

	WarningCanvasPanel->SetVisibility(ESlateVisibility::Collapsed);
}

void ULoginWidgetBase::OnLoginButton()
{
	AMainPC* PC = GetOwningPlayer<AMainPC>();
	if (PC)
	{
		FString FId = UserID->GetText().ToString();
		FString FPassword = Password->GetText().ToString();

		PC->SendLoginPacket(FId, FPassword);
	}
}

void ULoginWidgetBase::OnJoinButton()
{
	AMainPC* PC = GetOwningPlayer<AMainPC>();
	if (PC)
	{
		PC->ShowJoinWidget(true);
	}
}

void ULoginWidgetBase::OnShowPasswordButton()
{
	if (Password)
	{
		bool ShowPassword = Password->IsPassword;
		Password->SetIsPassword(!ShowPassword);

		FString ShowPasswordTexturePath;
		UTexture2D* ShowPasswordTexture;
		if (!ShowPassword)
		{
			ShowPasswordTexturePath = "/Script/Engine.Texture2D'/Game/Blueprints/Main/Assets/2p_Btn_Visivle.2p_Btn_Visivle'";
		}
		else
		{
			ShowPasswordTexturePath = "/Script/Engine.Texture2D'/Game/Blueprints/Main/Assets/2p_Btn_NotVisivle.2p_Btn_NotVisivle'";
		}

		ShowPasswordTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *ShowPasswordTexturePath));
		FButtonStyle  Style = ShowPasswordButton->GetStyle();
		Style.Normal.SetResourceObject(ShowPasswordTexture);
		Style.Hovered.SetResourceObject(ShowPasswordTexture);
		Style.Hovered.SetImageSize(Style.Normal.GetImageSize());
		Style.Pressed.SetResourceObject(ShowPasswordTexture);
		Style.Pressed.SetImageSize(Style.Normal.GetImageSize());
		ShowPasswordButton->SetStyle(Style);
	}
}

void ULoginWidgetBase::InitializeUI()
{
	UserID->SetText(FText::FromString(""));
	Password->SetText(FText::FromString(""));

	FString LoginTexturePath = "/Script/Engine.Texture2D'/Game/Blueprints/Main/Assets/4p_Btn_Log.4p_Btn_Log'";
	UTexture2D* LoginTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *LoginTexturePath));

	FString HoveredTexturePath = "/Script/Engine.Texture2D'/Game/Blueprints/Main/Assets/4p_Btn_Log_Impact.4p_Btn_Log_Impact'";
	UTexture2D* HoveredTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *HoveredTexturePath));

	FButtonStyle  Style = LoginButton->GetStyle();
	Style.Normal.SetResourceObject(LoginTexture);
	Style.Hovered.SetResourceObject(HoveredTexture);
	Style.Hovered.SetImageSize(Style.Normal.GetImageSize());
	Style.Pressed.SetResourceObject(HoveredTexture);
	Style.Pressed.SetImageSize(Style.Normal.GetImageSize());
	LoginButton->SetStyle(Style);

	WarningCanvasPanel->SetVisibility(ESlateVisibility::Collapsed);
}

void ULoginWidgetBase::ChangeLoginFailedUI()
{
	FString LoginFailedTexturePath = "/Script/Engine.Texture2D'/Game/Blueprints/Main/Assets/4p_Btn_Log_Warning.4p_Btn_Log_Warning'";
	UTexture2D* LoginFailedTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *LoginFailedTexturePath));
	
	FButtonStyle  Style = LoginButton->GetStyle();
	Style.Normal.SetResourceObject(LoginFailedTexture);
	Style.Hovered.SetResourceObject(LoginFailedTexture);
	Style.Hovered.SetImageSize(Style.Normal.GetImageSize());
	Style.Pressed.SetResourceObject(LoginFailedTexture);
	Style.Pressed.SetImageSize(Style.Normal.GetImageSize());
	LoginButton->SetStyle(Style);

	WarningCanvasPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
}