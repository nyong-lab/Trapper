// Fill out your copyright notice in the Description page of Project Settings.


#include "Dialog/DialogManager.h"
#include "Framework/TrapperPlayerController.h"
#include "Framework/TrapperGameState.h"
#include "Framework/TrapperGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Components/AudioComponent.h"
#include "TrapperProject.h"

// Sets default values
ADialogManager::ADialogManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UDataTable> DialogTable(TEXT("/Script/Engine.DataTable'/Game/Blueprints/Data/DT_DialogData.DT_DialogData'"));
	if (DialogTable.Succeeded() && DialogTable.Object)
	{
		DialogData = DialogTable.Object;
	}

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADialogManager::BeginPlay()
{
	Super::BeginPlay();
}

void ADialogManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

// Called every frame
void ADialogManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADialogManager::MulticastRPCPlayDialog_Implementation(int32 DialogCode)
{
	if (!PlayerController)
	{
		PlayerController = Cast<ATrapperPlayerController>(GetWorld()->GetFirstPlayerController());
	}

	//if (HasAuthority())
	//{
	//	FString Message = FString::Printf(TEXT("[Server] Play Dialog %d"), DialogCode);
	//	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, Message);
	//}
	//else
	//{
	//	FString Message = FString::Printf(TEXT("[Client] Play Dialog %d"), DialogCode);
	//	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, Message);
	//}

	PlayDialog(DialogCode);
}

void ADialogManager::MulticastRPCEndDialog_Implementation()
{
	EndDialog();
}

void ADialogManager::PlayDialog(int32 DialogCode)
{
	DialogIndex++;

	FString DialogDataName = FString::FromInt(DialogCode) + TEXT("_") + FString::FromInt(DialogIndex);
	FDialogInfo* Dialog = DialogData->FindRow<FDialogInfo>(*DialogDataName, FString());
	LastDialogCode = DialogCode;

	//UE_LOG(LogQuest, Warning, TEXT("Play Dialog %d_%d"), DialogCode, DialogIndex);

	if (Dialog)
	{
		// 이벤트 처리
		if (Dialog->EventCode.Num() != 0 && HasAuthority())
		{
			for (auto Code : Dialog->EventCode)
			{
				GetWorld()->GetGameState<ATrapperGameState>()->OnEventExecute.Broadcast(Code);
			}
		}
		
		float DialogTime = 0.f;

		// 사운드 재생
		if (IsValid(Dialog->VoiceFile))
		{
			AudioComponent->SetSound(Dialog->VoiceFile);
			AudioComponent->Play();
			DialogTime = Dialog->VoiceFile->Duration + VoiceOffset;
		}
		else
		{
			DialogTime = Dialog->Time;
		}

		// 대사 출력
		if (Dialog->Character == ECharacterName::Riana || Dialog->Character == ECharacterName::Diana)
		{
			if (bGodPlaying)
			{
				PlayerController->ShowGodDialog(false);
				bGodPlaying = false;
			}

			bPlayerPlaying = true;
			PlayerController->ShowPlayerDialog(true);
			PlayerController->SetPlayerText(GetCharacterName(Dialog->Character), Dialog->Dialog);
		}
		else
		{
			if (bPlayerPlaying)
			{
				PlayerController->ShowPlayerDialog(false);
				bPlayerPlaying = false;
			}

			// 0은 죽음의 신, 1은 인류의 신
			int8 index = (Dialog->Character == ECharacterName::Death) ? index = 0 : index = 1;

			bGodPlaying = true;
			PlayerController->ShowGodDialog(true);
			PlayerController->SetGodDialog(index, Dialog->Dialog);
		}
		
		if (Dialog->bIsEnd)
		{
			GetWorldTimerManager().SetTimer(DialogEndHandle, this,
				&ADialogManager::EndDialog, 1.0f, false, DialogTime);
		}
		else
		{
			GetWorldTimerManager().SetTimer(DialogEndHandle, this,
				&ADialogManager::ContinueDialog, 1.0f, false, DialogTime);
		}
	}
}

FString ADialogManager::GetCharacterName(ECharacterName Character)
{
	if (Character == ECharacterName::Riana)
	{
		return FString(TEXT("리아나"));
	}
	else if (Character == ECharacterName::Diana)
	{
		return FString(TEXT("디아나"));
	}
	else if (Character == ECharacterName::Death)
	{
		return FString(TEXT("죽음의 신"));
	}

	return FString();
}

void ADialogManager::EndDialog()
{
	DialogIndex = 0;

	if (bGodPlaying)
	{
		PlayerController->ShowGodDialog(false);
		bGodPlaying = false;
	}

	if (bPlayerPlaying)
	{
		PlayerController->ShowPlayerDialog(false);
		bPlayerPlaying = false;
	}

	if (AudioComponent && AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}
}

void ADialogManager::ContinueDialog()
{
	PlayDialog(LastDialogCode);
}

