// Fill out your copyright notice in the Description page of Project Settings.


#include "MainWidgetBase.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "MainPC.h"
#include "Kismet/GameplayStatics.h"
#include "Main/JoinWidgetBase.h"

void UMainWidgetBase::NativeConstruct()
{
	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &UMainWidgetBase::OnLoginButton);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UMainWidgetBase::OnJoinButton);
	}
}

void UMainWidgetBase::OnJoinButton()
{
	AMainPC* PC = GetOwningPlayer<AMainPC>();
	if (PC)
	{
		PC->ShowJoinWidget(true);
	}
}

void UMainWidgetBase::OnLoginButton()
{
	// ·Î±×ÀÎ
	AMainPC* PC = GetOwningPlayer<AMainPC>();
	if (PC)
	{
		PC->ShowLoginWidget(true);
	}
}