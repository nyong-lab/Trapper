// Fill out your copyright notice in the Description page of Project Settings.


#include "TrapperGameMode.h"
#include "Framework/TrapperPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"


#include "Quest/QuestManager.h"
#include "Sequence/SequenceManager.h"
#include "Dialog/DialogManager.h"

#include "GameElement/Spawner.h"
#include "EngineUtils.h"
#include "Character/TrapperPlayer.h"
#include "TrapperProject.h"
#include "Tower/Tower.h"
#include "Trap/TrabSnabEnviroment/TrapSnapTrigger.h"
#include "Level/TrapperScriptActor.h"
#include "Monster/BaseMonster.h"

ATrapperGameMode::ATrapperGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	GameStateClass = ATrapperGameState::StaticClass();

	static ConstructorHelpers::FObjectFinder<UDataTable> WaveTable(TEXT("/Script/Engine.DataTable'/Game/Blueprints/Data/DT_WaveData.DT_WaveData'"));
	if (WaveTable.Succeeded() && WaveTable.Object)
	{
		OriginalWaveData = WaveTable.Object;
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> DebugWaveTable(TEXT("/Script/Engine.DataTable'/Game/Blueprints/Data/DT_WaveData_Debug.DT_WaveData_Debug'"));
	if (DebugWaveTable.Succeeded() && DebugWaveTable.Object)
	{
		DebugWaveData = DebugWaveTable.Object;
	}
}

ATrapperGameMode::~ATrapperGameMode()
{

}

void ATrapperGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//if (!NewPlayer->IsLocalController())
	//{
	//	FTimerHandle TimerHandle;
	//	GetWorldTimerManager().SetTimer(TimerHandle, this, &ATrapperGameMode::ClientLoginAfterSetting, 2.0f, false, 1.0f);

	//	//CreateGuillotine();
	//}
}

void ATrapperGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (!Exiting->IsLocalController())
	{
		if (bServerPlayerTitleTravle)
		{
			UGameplayStatics::OpenLevel(this, "Title");
		}
	}
}

void ATrapperGameMode::BeginPlay()
{
	Super::BeginPlay();

	CreateManager();

	TrapperGameState = Cast<ATrapperGameState>(GetWorld()->GetGameState());
	TrapperGameState->OnEventExecute.AddUObject(this, &ATrapperGameMode::EventHandle);

	if (!bUseDebugWaveSheet && OriginalWaveData)
	{
		WaveData = OriginalWaveData;
	}
	else if (bUseDebugWaveSheet && DebugWaveData)
	{
		WaveData = DebugWaveData;
	}

	// Spawner 받아오기
	for (TActorIterator<AActor> MyActor(GetWorld()); MyActor; ++MyActor)
	{
		if (MyActor->ActorHasTag(TEXT("Spawner")))
		{
			Spanwer = Cast<ASpawner>(*MyActor);
		}
	}

	ALevelScriptActor* LevelScriptActor = GetWorld()->GetLevelScriptActor();
	TrapperLevel = Cast<ATrapperScriptActor>(LevelScriptActor);
}

void ATrapperGameMode::StartPlay()
{
	Super::StartPlay();
}

void ATrapperGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearAllTimersForObject(this);
}

void ATrapperGameMode::Tick(float DeltaTime)
{

}

void ATrapperGameMode::ClientLoginAfterSetting()
{

}

void ATrapperGameMode::SkipBeforeBoss()
{
	InitialItemSetting(InitialItemCount);
	QuestManager->SkipBeforeBoss();

	TrapperGameState->MaintenanceCount = 6;

	TrapperGameState->TowerUpgradeLevel = 6;
	TrapperGameState->OnRep_TowerUpgrade();

	TrapperGameState->Wave = 21;
	TrapperGameState->SubWave = 1;

	RemainingMonsterCountWave = 21;

	SetSkipIcon(false);

	GetThisWaveRemainingMonsterCount();
	SetGameProgress(EGameProgress::BossWave);

	EventHandle(525);
}

void ATrapperGameMode::CreateManager()
{
	if (!SequenceManager)
	{
		SequenceManager = GetWorld()->SpawnActor<ASequenceManager>(SequenceManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	if (!QuestManager)
	{
		QuestManager = GetWorld()->SpawnActor<AQuestManager>(QuestManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	if (!DialogManager)
	{
		DialogManager = GetWorld()->SpawnActor<ADialogManager>(DialogeManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}
}

void ATrapperGameMode::GameStart()
{
	if (!bSkipTutorial && !bSkipBeforeBoss)
	{
		// 첫 다이얼로그 재생
		EventHandle(500);
		TrapperLevel->MulticastPlayBackgroundSound(EBackgroundSound::Tutorial, 3.f);
	}
	else if (bSkipTutorial)
	{
		SkipTutorial();
	}
	else if (bSkipBeforeBoss)
	{
		SkipBeforeBoss();
	}
}

void ATrapperGameMode::EventHandle(int32 EventCode)
{
	//UE_LOG(LogQuest, Log, TEXT("Event Code : %d"), EventCode);

	switch (EventCode)
	{

		/// --------------------------------
		///				Event 
		/// --------------------------------

#pragma region Event
	// [튜토리얼] 정비 단계로 이동 (아이템 지급O)
	case 1:
	{
		SetGameProgress(EGameProgress::Maintenance);
		InitialItemSetting(InitialItemCount);
	}
	break;

	// [튜토리얼] 정비 단계로 이동 (아이템 지급X)
	case 2:
	{
		SetGameProgress(EGameProgress::Maintenance);
	}
	break;

	// [튜토리얼] 액터 정리
	case 3:
	{
		QuestManager->TutorialEndSetting();
	}
	break;

	// 정비시간 차감 시작
	case 4:
	{
		StartMaintenanceTime(true);

		if (TrapperGameState->MaintenanceCount > 0)
		{
			SetSkipIcon(true);
		}
	}
	break;

	// 정비 단계로 이동
	case 5:
	{
		SetGameProgress(EGameProgress::Maintenance);
	}
	break;

	// 신전 해방 단계 업그레이드
	case 6:
	{
		TrapperGameState->TowerUpgradeLevel++;
		TrapperGameState->OnRep_TowerUpgrade();

		// 타워 이펙트
		for (ATower* Tower : TActorRange<ATower>(GetWorld()))
		{
			Tower->MulticastUpgradeTower(TrapperGameState->TowerUpgradeLevel);
		}
	}
	break;

	// 웨이브 정보 숨김
	case 7:
	{
		for (ATrapperPlayerController* Player : TActorRange<ATrapperPlayerController>(GetWorld()))
		{
			Player->MulticastRPCShowWaveInfo(false);
		}
	}
	break;

	// 웨이브 단계로 이동 & 남은 몬스터 수 계산
	case 8:
	{
		GetThisWaveRemainingMonsterCount();
		TrapperGameState->Wave++;
		SetGameProgress(EGameProgress::BonusWave);
	}
	break;

	// 타워 관련 대사 및 타워 튜토리얼 대사 출력
	case 9:
	{
		TrapperGameState->bTowerTutorial = true;
		EventHandle(514);

		for (ATrapperPlayerController* Player : TActorRange<ATrapperPlayerController>(GetWorld()))
		{
			Player->MulticastRPCShowTutorialDialog(true, 2, TEXT("포탑을 지키기 위해 함정을 사용해 적들을 처치하세요."));
		}

		GetWorldTimerManager().SetTimer(UnShowDialogTimerHandle, this,
			&ATrapperGameMode::UnShowTutorialDialog, 1.f, false, 10.f);
	}
	break;

	// 보스 쫄따구 정리
	case 10:
	{
		KillCurrentMonster();
		TrapperGameState->MaintenanceCount = 6;
		//보스체력바활성화
		bIsBossStage = true;
		//UE_LOG(LogTemp, Warning, TEXT("Maintenance %d"), TrapperGameState->MaintenanceCount);
	}

	break;

	// 게임 클리어
	case 11:
	{
		SetGameProgress(EGameProgress::GameClear);
	}
	break;

	// 각성 및 각성 단계로 이동
	case 12:
	{
		//BossStage 종료 확인 변수
		bIsBossStage = false;

		TrapperGameState->UpgradePlayerAwaken(true);
		TrapperGameState->UpgradePlayerAwaken(false);

		SetGameProgress(EGameProgress::Awaken);
	}
	break;

	// 배경음악 중지
	case 13:
	{
		TrapperLevel->MulticastStopBackgroundSound(2.f);
	}
	break;

	// 함정 튜토리얼 텍스트
	case 14:
	{
		GetWorldTimerManager().SetTimer(MediaTimerHandle, this,
			&ATrapperGameMode::TrapTutorialTextChange, 1.f, false, 3.f);
	}
	break;

	// 스킵 아이콘 활성화
	case 15:
	{
		SetSkipIcon(true);
	}
	break;

	// 스킵 아이콘 비활성화
	case 16:
	{
		SetSkipIcon(false);
	}
	break;

	// 퀘스트 시작
	case 97:
	{
		QuestManager->DestroyQuestActor();
		QuestManager->QuestStart();

		for (ATrapperPlayerController* Player : TActorRange<ATrapperPlayerController>(GetWorld()))
		{
			Player->MulticastRPCShowQuestLayout(true);
		}
	}
	break;

	// 시퀀스 HUD 표시
	case 98:
	{
		for (ATrapperPlayerController* Player : TActorRange<ATrapperPlayerController>(GetWorld()))
		{
			Player->MulticastSequenceHudShow(true);
			//Player->DisableInput(Player);
		}
	}
	break;

	// 시퀀스 HUD 숨김
	case 99:
	{
		for (ATrapperPlayerController* Player : TActorRange<ATrapperPlayerController>(GetWorld()))
		{
			Player->MulticastSequenceHudShow(false);
			//Player->EnableInput(Player);
		}
	}
	break;

#pragma endregion Event

	/// --------------------------------
	///				Sequence 
	/// --------------------------------

#pragma region Sequence

	case 202:
	{
		SequenceManager->MulticastRPCPlaySequence(ESequenceType::FirstWave);
	}
	break;

	case 203:
	{
		GetWorldTimerManager().SetTimer(MediaTimerHandle, this,
			&ATrapperGameMode::PlayAppearBoss, 1.f, false, 4.f);
	}
	break;

	case 204:
	{
		GetWorldTimerManager().SetTimer(MediaTimerHandle, this,
			&ATrapperGameMode::PlayStartBonusWave, 1.f, false, 4.f);
	}
	break;

	case 206:
	{
		SequenceManager->MulticastRPCPlaySequence(ESequenceType::Ending);

		for (ATrapperPlayer* Player : TActorRange<ATrapperPlayer>(GetWorld()))
		{
			Player->MulticastRPCStopAwakenSound();
		}
	}
	break;

#pragma endregion Sequence

	default:
		break;

#pragma region Cheat

		// 정비 1 (튜토리얼 마지막 퀘스트 업그레이드 선택 전)
	case 300:
	{
		if (TrapperGameState->MaintenanceCount > -1)
		{
			return;
		}

		SetSkipIcon(true);
		QuestManager->SetQuest(6);
		QuestManager->QuestComplete();
		for (ATrapperPlayerController* Player : TActorRange<ATrapperPlayerController>(GetWorld()))
		{
			Player->MulticastRPCShowQuestLayout(true);
		}

		DialogManager->MulticastRPCEndDialog();
		UnShowTutorialDialog();
		TrapperGameState->MaintenanceCount = 0;
	}
	break;
	// 정비 2 (업그레이드 선택 전)
	case 301:
		ToSkipMaintanenctCheat(1, 5, 12, 515);
		break;
		// 정비 3 (업그레이드 선택 전)
	case 302:
		ToSkipMaintanenctCheat(2, 9, 15, 518);
		break;
		// 정비 4 (업그레이드 선택 전)
	case 303:
		ToSkipMaintanenctCheat(3, 13, 18, 520);
		break;
		// 정비 5 (업그레이드 선택 전)
	case 304:
		ToSkipMaintanenctCheat(4, 17, 21, 522);
		break;
		// 정비 6 (업그레이드 선택 전)
	case 305:
		ToSkipMaintanenctCheat(5, 21, 24, 524);
		break;
		// After Boss (각성 업그레이드 선택 전)
	case 306:
	{
		if (TrapperGameState->CurrentGameProgress == EGameProgress::BonusWave ||
			TrapperGameState->CurrentGameProgress == EGameProgress::Awaken)
		{
			return;
		}

		for (ATrapperPlayerController* Player : TActorRange<ATrapperPlayerController>(GetWorld()))
		{
			Player->MulticastRPCShowQuestLayout(false);
		}

		// 게임 상태별 처리
		if (TrapperGameState->CurrentGameProgress == EGameProgress::Tutorial)
		{
			QuestManager->DestroyQuestActor();
			QuestManager->TutorialEndSetting();
			InitialItemSetting(InitialItemCount);
			TrapperGameState->S2SReactivateAllMagneticPillar();
		}
		else if (TrapperGameState->CurrentGameProgress == EGameProgress::Maintenance)
		{
			GetWorldTimerManager().ClearAllTimersForObject(this);
		}
		else if (TrapperGameState->CurrentGameProgress == EGameProgress::Wave)
		{
			GetWorldTimerManager().ClearAllTimersForObject(this);
			KillCurrentMonster();
		}

		TrapperGameState->bTowerTutorial = true;

		// 다이얼로그 종료
		DialogManager->MulticastRPCEndDialog();

		// 업그레이드 상태 (UI 표시)
		TrapperGameState->TowerUpgradeLevel = 6;
		TrapperGameState->OnRep_TowerUpgrade();

		// 웨이브 설정
		TrapperGameState->Wave = 22;
		TrapperGameState->SubWave = 1;
		RemainingMonsterCountWave = 22;

		TrapperGameState->CurrentMonsterCount = 0;
		TrapperGameState->RemainingMonsterCount = 0;
		GetThisWaveRemainingMonsterCount();

		TrapperGameState->MaintenanceCount = 6;
		SetGameProgress(EGameProgress::BonusWave);
		QuestManager->SetQuest(25);
		SetSkipIcon(false);

		EventHandle(7);
		EventHandle(528);
	}
	break;
	// GameRestart
	case 307:
	{
		GetWorld()->ServerTravel(TEXT("Opening"));
	}
	break;
	// AddBone
	case 308:
		InitialItemSetting(100);
		break;
		// Invincibility
	case 309:
	{
		TrapperGameState->bIsInvincibilityMode = ~TrapperGameState->bIsInvincibilityMode;

		for (ATrapperPlayerController* Player : TActorRange<ATrapperPlayerController>(GetWorld()))
		{
			Player->MulticastShowInvincibilityModeText(TrapperGameState->bIsInvincibilityMode);
		}
	}
	break;
		// MonsterKill
	case 310:
		KillCurrentMonster();
		break;
	}

#pragma endregion Chat


	/// --------------------------------
	///				Dialog 
	/// --------------------------------

	if (EventCode >= 500 && EventCode < 600)
	{
		DialogManager->MulticastRPCPlayDialog(EventCode);
	}
}

void ATrapperGameMode::SetGameProgress(EGameProgress Progress)
{
	if (!TrapperGameState)
	{
		UE_LOG(LogQuest, Warning, TEXT("No Valid Trapper Game State"));
	}

	EGameProgress PreviousProgress = TrapperGameState->CurrentGameProgress;

	if (PreviousProgress != Progress)
	{
		TrapperGameState->CurrentGameProgress = Progress;
		TrapperGameState->OnRep_GameProgressSetting();
	}

	switch (Progress)
	{
	case EGameProgress::Tutorial:
	{
		//UE_LOG(LogTemp, Warning, TEXT("Game Progress : Tutorial"));
	}
	break;
	case EGameProgress::Maintenance:
	{
		//UE_LOG(LogTemp, Warning, TEXT("Game Progress : Maintenance"));

		TrapperLevel->MulticastPlayBackgroundSound(EBackgroundSound::Maintenance, 2.f);
		MaintenanceStart();
	}
	break;
	case EGameProgress::Wave:
	{
		//UE_LOG(LogTemp, Warning, TEXT("Game Progress : Wave"));

		if (TrapperGameState->Wave != 1)
		{
			TrapperLevel->MulticastPlayBackgroundSound(EBackgroundSound::Wave, 2.f);
		}

		WaveStart();
	}
	break;
	case EGameProgress::BossWave:
		//UE_LOG(LogTemp, Warning, TEXT("Game Progress : BossWave"));
		WaveStart();
		break;
	case EGameProgress::BonusWave:
		//UE_LOG(LogTemp, Warning, TEXT("Game Progress : BonusWave"));
		TrapperLevel->MulticastPlayBackgroundSound(EBackgroundSound::BonusWave, 2.f);
		WaveStart();
		break;
	case EGameProgress::Awaken:
		TrapperLevel->MulticastPlayBackgroundSound(EBackgroundSound::Awaken, 2.f);
		//UE_LOG(LogTemp, Warning, TEXT("Game Progress : Awaken"));
		break;
	case EGameProgress::GameClear:
		//UE_LOG(LogTemp, Warning, TEXT("Game Progress : GameClear"));
		GameClear(true);
		break;
	case EGameProgress::GameOver:
		//UE_LOG(LogTemp, Warning, TEXT("Game Progress : GameOver"));
		GameClear(false);
		break;
	default:
		break;
	}

}

EGameProgress ATrapperGameMode::GetCurrentGameProgress()
{
	return TrapperGameState->CurrentGameProgress;
}

void ATrapperGameMode::InitialItemSetting(int Value)
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATrapperPlayer::StaticClass(), OutActors);

	for (const auto& Pawn : OutActors)
	{
		ATrapperPlayer* TrapperPlayer = Cast<ATrapperPlayer>(Pawn);

		if (TrapperPlayer)
		{
			TrapperPlayer->AddBoneItem(Value);
		}
	}
}

void ATrapperGameMode::MaintenanceStart()
{
	// 정비 시간 설정
	TrapperGameState->SetMaintenanceTime(MaintenanceTime);
	TrapperGameState->MaintenanceCount++;

	// 웨이브에 출현할 몬스터 계산
	GetThisWaveRemainingMonsterCount();
}


void ATrapperGameMode::StartMaintenanceTime(bool Value)
{
	TrapperGameState->bMaintenanceInProgress = Value;
	TrapperGameState->OnRep_ChangeMaintenanceState();

	if (!Value) return;

	GetWorldTimerManager().SetTimer(MaintenanceHandle, this, &ATrapperGameMode::MaintenanceTimer, 1.f, true, 1.f);
}

void ATrapperGameMode::MaintenanceTimer()
{
	TrapperGameState->SetMaintenanceTime(--TrapperGameState->MaintenanceTimeLeft);

	// 정비시간 스킵하거나 끝났을 경우
	if (bSkipMaintenance || TrapperGameState->MaintenanceTimeLeft <= 0.f)
	{
		SkipMaintenance();
		GetWorldTimerManager().ClearTimer(MaintenanceHandle);
	}
}

void ATrapperGameMode::WaveStart()
{
	UE_LOG(LogQuest, Warning, TEXT("Wave %d-%d"), TrapperGameState->Wave, TrapperGameState->SubWave);

	if (GetWaveData(CurrentWaveData))
	{
		SpawnMonster(CurrentWaveData);
	}
	else return;

	TrapperGameState->bWaveInProgress = true;
	TrapperGameState->SetWaveTime(CurrentWaveData.NextWaveLeftTime);
	TrapperGameState->SetWaveInfo();

	if (CurrentWaveData.bLastLargeWave == true)
	{
		TrapperGameState->SubWave = 1;
		TrapperGameState->bWaveInProgress = false;
		TrapperGameState->SetWaveTime(0.f);
		return;
	}

	GetWorldTimerManager().SetTimer(WaveHandle, this, &ATrapperGameMode::WaveTimer, 1.f, true, 1.f);
}

void ATrapperGameMode::WaveTimer()
{
	TrapperGameState->SetWaveTime(--TrapperGameState->WaveTimeLeft);

	if (TrapperGameState->WaveTimeLeft <= 0.f)
	{
		/// Wave --------------------------------------------
		if (GetCurrentGameProgress() == EGameProgress::Wave ||
			GetCurrentGameProgress() == EGameProgress::BossWave)
		{
			// 다음 웨이브로 넘어감
			if (CurrentWaveData.bLastSubWave)
			{
				TrapperGameState->Wave++;
				TrapperGameState->SubWave = 1;
			}
			// 서브 웨이브 계속 진행
			else
			{
				TrapperGameState->SubWave++;
			}

			GetWorldTimerManager().ClearTimer(WaveHandle);
			WaveStart();
		}

		/// Bonus Wave ------------------------------------------
		if (GetCurrentGameProgress() == EGameProgress::BonusWave
			|| GetCurrentGameProgress() == EGameProgress::Awaken)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Go Timer"));

			if (CurrentWaveData.bLastSubWave)
			{
				GetWorldTimerManager().ClearTimer(WaveHandle);
				//UE_LOG(LogTemp, Warning, TEXT("Clear Timer"));
				return;
			}

			if (TrapperGameState->CurrentMonsterCount <= 0)
			{
				TrapperGameState->SubWave++;
				GetWorldTimerManager().ClearTimer(WaveHandle);
				WaveStart();
				//UE_LOG(LogTemp, Warning, TEXT("SubWave %d"), TrapperGameState->SubWave);
			}
		}
	}
}

void ATrapperGameMode::GetThisWaveRemainingMonsterCount()
{
	uint32 k = 1;

	while (true)
	{
		FString WaveText = TEXT("Wave") + FString::FromInt(RemainingMonsterCountWave) + TEXT("_") + FString::FromInt(k);
		FWaveInfo* Data = WaveData->FindRow<FWaveInfo>(*WaveText, FString());
		if (!Data)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("Wave Data Error"));
			return;
		}

		uint32 TotalMonster = 0;

		TotalMonster += Data->Skeleton;
		TotalMonster += Data->Mummy;
		TotalMonster += Data->Zombie;
		TotalMonster += Data->Debuffer;
		TotalMonster += Data->Boss;

		TrapperGameState->ChangeRemainingMonsterCount(TotalMonster);
		UE_LOG(LogQuest, Warning, TEXT("This Wave %d-%d, Total Spawn Monster %d"), RemainingMonsterCountWave, k, TotalMonster);

		if (Data->bLastLargeWave)
		{
			RemainingMonsterCountWave++;
			return;
		}
		else if (Data->bLastSubWave)
		{
			RemainingMonsterCountWave++;
			k = 1;
		}
		else
		{
			k++;
		}
	}

}

void ATrapperGameMode::KillCurrentMonster()
{
	for (auto Monster : Spanwer->SpawnMonsterList)
	{
		UGameplayStatics::ApplyDamage(Monster, Monster->MaxHP, GetInstigatorController(), this, UDamageType::StaticClass());
	}
}

bool ATrapperGameMode::GetWaveData(FWaveInfo& OutData)
{
	FString WaveText = TEXT("Wave") + FString::FromInt(TrapperGameState->Wave) + TEXT("_") + FString::FromInt(TrapperGameState->SubWave);
	FWaveInfo* Data = WaveData->FindRow<FWaveInfo>(*WaveText, FString());
	if (!Data) return false;

	OutData.bLastSubWave = Data->bLastSubWave;
	OutData.bLastLargeWave = Data->bLastLargeWave;
	OutData.Skeleton = Data->Skeleton;
	OutData.Mummy = Data->Mummy;
	OutData.Zombie = Data->Zombie;
	OutData.Debuffer = Data->Debuffer;
	OutData.Boss = Data->Boss;
	OutData.NextWaveLeftTime = Data->NextWaveLeftTime;
	return true;
}

void ATrapperGameMode::SpawnMonster(struct FWaveInfo& OutData)
{
	/// *********** Create Monster ***********
	if (!bSpawnMonster) return;

	UE_LOG(LogQuest, Warning, TEXT("Create Monster - Skeleton %d / Mummy %d / Zombie %d / Debuffer %d / Boss %d"),
		OutData.Skeleton, OutData.Mummy, OutData.Zombie, OutData.Debuffer, OutData.Boss);

	if (IsValid(Spanwer))
	{
		Spanwer->SpawnMonsters(OutData.Skeleton, OutData.Mummy, OutData.Zombie, OutData.Debuffer, OutData.Boss);
		//UE_LOG(LogTemp, Warning, TEXT("Check"));
	}

	//UE_LOG(LogQuest, Warning, TEXT("Delete Monster : %d"), DeleteMonsterCount);
}

void ATrapperGameMode::GameClear(bool IsGameClear)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ATrapperPlayerController* PlayerController = Cast<ATrapperPlayerController>(Iterator->Get());
		if (PlayerController)
		{
			//PlayerController->MulticastRPCGameEndScreenSetting(IsGameClear);

			TrapperGameState->UpdateScoreHUD();
			PlayerController->ClientRPCGameEndShowScore(IsGameClear);
		}
	}
}

void ATrapperGameMode::SkipTutorial()
{
	//UE_LOG(LogQuest, Warning, TEXT("Skip Tutorial"));

	InitialItemSetting(InitialItemCount);
	QuestManager->SkipTutorial();
	SetSkipIcon(false);

	SetGameProgress(EGameProgress::Maintenance);

	TrapperGameState->OnEventExecute.Broadcast(6);
	TrapperGameState->OnEventExecute.Broadcast(512);

	TrapperGameState->S2SReactivateAllMagneticPillar();
}

void ATrapperGameMode::SkipMaintenance()
{
	bSkipMaintenance = false;

	TrapperGameState->bMaintenanceInProgress = false;
	SetSkipIcon(false);

	TrapperGameState->OnQuestExecute.Broadcast(99, true);

	TrapperGameState->Wave++;

	if (TrapperGameState->Wave >= 20)
	{
		SetGameProgress(EGameProgress::BossWave);
	}
	else
	{
		SetGameProgress(EGameProgress::Wave);
	}
}

void ATrapperGameMode::ToSkipMaintanenctCheat(
	int MaintenanceCount,
	int WaveCount,
	int QuestIndex,
	int DialogIndex)
{
	// 같거나 낮은 단계로 스킵 방지
	if (TrapperGameState->MaintenanceCount > MaintenanceCount - 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skip Failed / Current Count %d"), TrapperGameState->MaintenanceCount);
		return;
	}

	for (ATrapperPlayerController* Player : TActorRange<ATrapperPlayerController>(GetWorld()))
	{
		Player->MulticastRPCShowQuestLayout(false);
	}

	// 게임 상태별 처리
	if (TrapperGameState->CurrentGameProgress == EGameProgress::Tutorial)
	{
		QuestManager->DestroyQuestActor();
		QuestManager->TutorialEndSetting();
		InitialItemSetting(InitialItemCount);
		TrapperGameState->S2SReactivateAllMagneticPillar();
	}
	else if (TrapperGameState->CurrentGameProgress == EGameProgress::Maintenance)
	{
		GetWorldTimerManager().ClearAllTimersForObject(this);
	}
	else if (TrapperGameState->CurrentGameProgress == EGameProgress::Wave)
	{
		GetWorldTimerManager().ClearAllTimersForObject(this);
		KillCurrentMonster();
	}

	TrapperGameState->bTowerTutorial = true;

	// 다이얼로그 종료
	DialogManager->MulticastRPCEndDialog();
	UnShowTutorialDialog();

	// 업그레이드 상태 (UI 표시)
	TrapperGameState->TowerUpgradeLevel = MaintenanceCount;
	TrapperGameState->OnRep_TowerUpgrade();

	// 웨이브 설정
	TrapperGameState->Wave = WaveCount - 1;
	TrapperGameState->SubWave = 1;
	RemainingMonsterCountWave = WaveCount;

	TrapperGameState->CurrentMonsterCount = 0;
	TrapperGameState->RemainingMonsterCount = 0;
	SetGameProgress(EGameProgress::Maintenance);
	TrapperGameState->MaintenanceCount = MaintenanceCount;
	//UE_LOG(LogTemp, Warning, TEXT("Skip Success / Change Count %d"), TrapperGameState->MaintenanceCount);

	// 퀘스트 설정
	QuestManager->SetQuest(QuestIndex - 2);

	// 대사 출력
	EventHandle(DialogIndex);
}

void ATrapperGameMode::UnShowTutorialDialog() // false
{
	for (ATrapperPlayerController* Player : TActorRange<ATrapperPlayerController>(GetWorld()))
	{
		Player->MulticastRPCShowTutorialDialog(false);
	}
}

void ATrapperGameMode::PlayStartBonusWave()
{
	SequenceManager->MulticastRPCPlaySequence(ESequenceType::StartBonusWave);
}

void ATrapperGameMode::TrapTutorialTextChange()
{
	for (ATrapperPlayerController* Player : TActorRange<ATrapperPlayerController>(GetWorld()))
	{
		Player->MulticastSetGodDialog(2, TEXT("포탑으로 몰려오는 몬스터들을 효율적으로 처치할 수 있도록 함정을 설치하세요."));
	}

	GetWorldTimerManager().SetTimer(UnShowDialogTimerHandle, this,
		&ATrapperGameMode::UnShowTutorialDialog, 1.f, false, 3.f);
}

void ATrapperGameMode::PlayAppearBoss()
{
	SequenceManager->MulticastRPCPlaySequence(ESequenceType::AppearBoss);
}

void ATrapperGameMode::SetSkipIcon(bool Value)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ATrapperPlayerController* PlayerController = Cast<ATrapperPlayerController>(Iterator->Get());
		if (PlayerController)
		{
			PlayerController->MulticastRPCSetSkipIcon(Value);
		}
	}
}

void ATrapperGameMode::OnTrapperLevelLoaded()
{
	CreateGuillotine();
}

void ATrapperGameMode::CreateGuillotine()
{
	FString LevelName = GetWorld()->GetMapName();
	//UE_LOG(LogTrap, Warning, TEXT("Current Level name: %s"), *LevelName);

	LevelName.RemoveFromStart(FString("UEDPIE_0_"));
	//UE_LOG(LogTrap, Warning, TEXT("Fixed Level Name : %s"), *LevelName);

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("Level Name : %s"), *LevelName));

	if (LevelName == "Trapper" || LevelName == "UEDPIE_0_Trapper" || LevelName == "UEDPIE_1_Trapper")
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, FString::Printf(TEXT("Good")));
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, FString::Printf(TEXT("Level Name : %s"), *LevelName));

		for (const auto& SnapTrigger : TrapSnapTriggerArray)
		{
			SnapTrigger->SpawnSnapGuillotine();
			/*SnapTrigger->ForceNetRelevant();
			SnapTrigger->ForceNetUpdate();*/
		}
	}
}



