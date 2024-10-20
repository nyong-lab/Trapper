// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/GameResultHUD.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UGameResultHUD::NativeConstruct()
{
	Super::NativeConstruct();

	BackTitleButton->OnClicked.AddDynamic(this, &UGameResultHUD::BackTitle);
}

void UGameResultHUD::BackTitle()
{
	//UE_LOG(LogTemp, Warning, TEXT("Back To Title"));
	UGameplayStatics::OpenLevel(this, TEXT("Title"));

	GetWorld()->GetFirstPlayerController()->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("접속 종료")));
}

void UGameResultHUD::SetGameEndText(bool IsGameClear)
{
	if (IsGameClear)
	{
		FString ClearText(TEXT("Demo Clear"));
		FString ClearDescriptionText(TEXT("플레이 해주셔서 감사합니다.\n\n다음 업데이트를 기대해주세요!"));

		MainText->SetText(FText::FromString(ClearText));
		MainText->SetColorAndOpacity(FSlateColor(FLinearColor(206.f / 255.f, 110.f / 255.f, 0.f / 255.f, 1.f)));
		DescriptionText->SetText(FText::FromString(ClearDescriptionText));
		ToBeContinuedText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		FString GameOverText(TEXT("Game Over"));
		FString GameOverDescriptionText(TEXT("돌아가기 버튼을 누르면 타이틀로 이동합니다."));

		MainText->SetText(FText::FromString(GameOverText));
		MainText->SetColorAndOpacity(FSlateColor(FLinearColor(207.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1.f)));
		DescriptionText->SetText(FText::FromString(GameOverDescriptionText));
		ToBeContinuedText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}
