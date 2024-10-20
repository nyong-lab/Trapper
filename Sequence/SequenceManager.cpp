// Fill out your copyright notice in the Description page of Project Settings.


#include "Sequence/SequenceManager.h"
#include "Kismet/GameplayStatics.h"
#include "TrapperProject.h"
#include "Framework/TrapperGameState.h"
#include "Framework/TrapperGameMode.h"
#include "Framework/TrapperPlayerController.h"
#include "Level/TrapperScriptActor.h"

#include "MediaPlayer.h"
#include "MediaSource.h"
#include "MediaTexture.h"

// Sets default values
ASequenceManager::ASequenceManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	static ConstructorHelpers::FObjectFinder<UMediaPlayer> MediaPlayerFinder(TEXT("/Script/MediaAssets.MediaPlayer'/Game/Movies/TrapperMoviePlayer.TrapperMoviePlayer'"));
	if (MediaPlayerFinder.Succeeded())
	{
		MoviePlayer = MediaPlayerFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMediaTexture> MediaTextureFinder(TEXT("/Script/MediaAssets.MediaTexture'/Game/Movies/TrapperMoviePlayer_Video.TrapperMoviePlayer_Video'"));
	if (MediaTextureFinder.Succeeded())
	{
		UE_LOG(LogTemp, Warning, TEXT("Movie Texture"));
		MovieTexture = MediaTextureFinder.Object;
	}

	// Media Source
	static ConstructorHelpers::FObjectFinder<UMediaSource> FirstWave(TEXT("/Script/MediaAssets.FileMediaSource'/Game/Movies/First_Wave.First_Wave'"));
	if (FirstWave.Succeeded())
	{
		MoviePlaylist.Add(FirstWave.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMediaSource> AppearBoss(TEXT("/Script/MediaAssets.FileMediaSource'/Game/Movies/Boss.Boss'"));
	if (AppearBoss.Succeeded())
	{
		MoviePlaylist.Add(AppearBoss.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMediaSource> BonusWave(TEXT("/Script/MediaAssets.FileMediaSource'/Game/Movies/Bonus.Bonus'"));
	if (BonusWave.Succeeded())
	{
		MoviePlaylist.Add(BonusWave.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMediaSource> Ending(TEXT("/Script/MediaAssets.FileMediaSource'/Game/Movies/Ending.Ending'"));
	if (Ending.Succeeded())
	{
		MoviePlaylist.Add(Ending.Object);
	}
}

// Called when the game starts or when spawned
void ASequenceManager::BeginPlay()
{
	Super::BeginPlay();

	MoviePlayer->SetLooping(false);
	MoviePlayer->PlayOnOpen = false;

	MoviePlayer->OnMediaOpened.AddDynamic(this, &ASequenceManager::HandleMediaOpened);

	ALevelScriptActor* LevelScriptActor = GetWorld()->GetLevelScriptActor();
	TrapperLevel = Cast<ATrapperScriptActor>(LevelScriptActor);
}

void ASequenceManager::HandleMediaOpened(FString OpenedUrl)
{
	if (MoviePlayer && MoviePlayer->IsReady())
	{
		MoviePlayer->Play();  // 미디어가 정상적으로 열린 후 재생 시작
	}

	if (HasAuthority())
	{
		ATrapperGameState* TrapperGameState = GetWorld()->GetGameState<ATrapperGameState>();
		TrapperGameState->OnEventExecute.Broadcast(98);
	}
}

void ASequenceManager::MovieClear()
{
	MoviePlayer->Close();
	MoviePlayer->OnEndReached.Clear();
	MovieTexture->UpdateResource();
}

void ASequenceManager::MulticastRPCPlaySequence_Implementation(ESequenceType Type)
{
	//UE_LOG(LogTemp, Warning, TEXT("Play Movie %d"), static_cast<uint32>(Type));

	CurrentMovie = Type;

	MoviePlayer->Close();
	MoviePlayer->OpenSource(MoviePlaylist[static_cast<uint32>(Type)]);

	switch (Type)
	{
	case ESequenceType::FirstWave:
		MoviePlayer->OnEndReached.AddDynamic(this, &ASequenceManager::FirstWaveFinished);
		break;
	case ESequenceType::AppearBoss:
		MoviePlayer->OnEndReached.AddDynamic(this, &ASequenceManager::AppearBossFinished);
		break;
	case ESequenceType::StartBonusWave:
		MoviePlayer->OnEndReached.AddDynamic(this, &ASequenceManager::BonusWaveFinished);
		break;
	case ESequenceType::Ending:
		MoviePlayer->OnEndReached.AddDynamic(this, &ASequenceManager::EndingFinished);
		break;
	default:
		break;
	}
}


/// -------------------------------------------------------------
///				시퀀스 플레이 후 작동해야 하는 것들
/// -------------------------------------------------------------

void ASequenceManager::FirstWaveFinished()
{
	MovieClear();

	if (HasAuthority())
	{
		TrapperLevel->MulticastPlayBackgroundSound(EBackgroundSound::Wave, 2.f);
	}

	// 웨이브 컷신 이후 다이얼로그 재생
	ATrapperGameState* TrapperGameState = GetWorld()->GetGameState<ATrapperGameState>();
	TrapperGameState->OnEventExecute.Broadcast(99);
	TrapperGameState->OnEventExecute.Broadcast(513);
}

void ASequenceManager::AppearBossFinished()
{
	MovieClear();

	if (HasAuthority())
	{
		TrapperLevel->MulticastPlayBackgroundSound(EBackgroundSound::BossWave, 2.f);
	}

	ATrapperGameState* TrapperGameState = GetWorld()->GetGameState<ATrapperGameState>();
	TrapperGameState->OnEventExecute.Broadcast(99);
	TrapperGameState->OnEventExecute.Broadcast(526);
}

void ASequenceManager::BonusWaveFinished()
{
	MovieClear();

	// 보너스 웨이브 컷신 이후 다이얼로그 재생
	ATrapperGameState* TrapperGameState = GetWorld()->GetGameState<ATrapperGameState>();
	TrapperGameState->OnEventExecute.Broadcast(99);
	TrapperGameState->OnEventExecute.Broadcast(528);
}

void ASequenceManager::EndingFinished()
{
	MovieClear();

	if (HasAuthority())
	{
		TrapperLevel->MulticastPlayBackgroundSound(EBackgroundSound::GameClear, 2.f);
	}

	ATrapperGameState* TrapperGameState = GetWorld()->GetGameState<ATrapperGameState>();
	TrapperGameState->OnEventExecute.Broadcast(99);
	TrapperGameState->OnEventExecute.Broadcast(11);
}

