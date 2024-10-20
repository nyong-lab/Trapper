// Fill out your copyright notice in the Description page of Project Settings.


#include "Opening/OpeningPC.h"
#include "Opening/OpeningWidgetBase.h"

void AOpeningPC::BeginPlay()
{
	if (IsLocalController() && OpeningWidgetClass)
	{
		OpeningWidgetObject = CreateWidget<UOpeningWidgetBase>(this, OpeningWidgetClass);
		OpeningWidgetObject->AddToViewport();
	}
}
