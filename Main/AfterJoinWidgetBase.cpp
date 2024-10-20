// Fill out your copyright notice in the Description page of Project Settings.


#include "Main/AfterJoinWidgetBase.h"
#include "Components/Button.h"
#include "MainPC.h"

void UAfterJoinWidgetBase::NativeConstruct()
{
	if (GotoLoginButton)
	{
		GotoLoginButton->OnClicked.AddDynamic(this, &UAfterJoinWidgetBase::OnGotoLoginButton);
	}
}

void UAfterJoinWidgetBase::OnGotoLoginButton()
{
	AMainPC* PC = GetOwningPlayer<AMainPC>();
	PC->ShowLoginWidget(true);
}