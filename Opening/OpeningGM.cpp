// Fill out your copyright notice in the Description page of Project Settings.


#include "Opening/OpeningGM.h"

void AOpeningGM::ServerTravelToTrapper_Implementation()
{
	GetWorld()->ServerTravel(TEXT("Trapper"));
}
