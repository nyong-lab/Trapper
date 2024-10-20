// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/AI/Task/BTTask_DestroyActor.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameElement/Spawner.h"

#include "Monster/PathTarget.h"
#include "Monster/DebufferPathTarget.h"
#include "Monster/ADC.h"
#include "Monster/CDC.h"
#include "Monster/DetectedMonster.h"
#include "Monster/Debuffer.h"
#include "Monster/BaseMonster.h"
#include "Monster/Boss/Boss_Phase1.h"
#include "Monster/Boss/Boss_Phase2.h"
#include "Framework/TrapperGameMode.h"
#include "Framework/TrapperGameState.h"
#include "Monster/AI/BaseMonsterAIController.h"
#include "Monster/AI/DebufferMonsterAIController.h"

EBTNodeResult::Type UBTTask_DestroyActor::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMenory)
{
	

	ATrapperGameMode* TrapperGameMode = Cast<ATrapperGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	ABaseMonsterAIController* AIC = Cast<ABaseMonsterAIController>(OwnerComp.GetAIOwner());
	
	if (IsValid(AIC))
	{
		APawn* Pawn = AIC->GetPawn();
		if (Pawn)
		{
			TObjectPtr<ABaseMonster> Monster = Cast<ABaseMonster>(Pawn);

			/*for (int i = 0; i < TrapperGameMode->Spanwer->SpawnMonsterList.Num(); i++)
			{
				UE_LOG(LogTemp, Warning, TEXT("MonsterList : %s"), *TrapperGameMode->Spanwer->SpawnMonsterList[i].GetName());
			}*/
			
			//UE_LOG(LogTemp, Warning, TEXT("Remove Monster: %s"), *Monster->GetName());

			if (IsValid(Monster))
			{
				/// Todo : RPC로 다 던져버리기 근데 이걸로도 뭔가 뭔가던데
				//Monster->SetActorLocation(Monster->StartPoint);
				//Monster->TeleportTo(Monster->StartPoint, Monster->GetActorRotation());
				Monster->ServerRPCTeleportToDie();
				Monster->bIsSpawn = false;
			
				//Path Clear
				for (int i = 0; i < AIC->PathTargets.Num(); i++)
				{
					//UE_LOG(LogTemp, Warning, TEXT("Remove Success"));
					AIC->PathTargets[i]->CheckTarget.Remove(Monster);
				}

				

				//������ �ʱ�ȭ
				Monster->CurrentHP = Monster->MaxHP;
				Monster->CurrentTarget = 0;
				Monster->MonsterState = EMonsterStateType::GoToTower;
				Monster->GetCharacterMovement()->MaxWalkSpeed = Monster->MonsterSpeed;
				Monster->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
				//Monster->GetMesh()->SetVisibility(false);
				
				if (IsValid(TrapperGameMode->Spanwer))
				{
					
					if (Monster->HasAuthority())
					{
						//UE_LOG(LogTemp, Warning, TEXT("Check Die : %s"), *Monster->GetName());
						//Objectpool
						if (Cast<AADC>(Monster))
						{
							if (TrapperGameMode->Spanwer->ADCList.Find(Cast<AADC>(Monster)) == INDEX_NONE)
							{
								TrapperGameMode->Spanwer->ADCList.Add(Cast<AADC>(Monster));
								GetWorld()->GetGameState<ATrapperGameState>()->ChangeMonsterCount(-1);
							}

							//UE_LOG(LogTemp, Warning, TEXT("ADC Die"));
						}
						if (Cast<ACDC>(Monster))
						{
							if (TrapperGameMode->Spanwer->CDCList.Find(Cast<ACDC>(Monster)) == INDEX_NONE)
							{
								TrapperGameMode->Spanwer->CDCList.Add(Cast<ACDC>(Monster));
								GetWorld()->GetGameState<ATrapperGameState>()->ChangeMonsterCount(-1);
							}

							//UE_LOG(LogTemp, Warning, TEXT("CDCList Size : %d"), TrapperGameMode->Spanwer->CDCList.Num());
						}
						if (Cast<ADetectedMonster>(Monster))
						{
							if (TrapperGameMode->Spanwer->DetectedList.Find(Cast<ADetectedMonster>(Monster)) == INDEX_NONE)
							{
								TrapperGameMode->Spanwer->DetectedList.Add(Cast<ADetectedMonster>(Monster));
								GetWorld()->GetGameState<ATrapperGameState>()->ChangeMonsterCount(-1);
							}

							//UE_LOG(LogTemp, Warning, TEXT("CDC Die"));
						}
						if (Cast<ADebuffer>(Monster))
						{
							if (TrapperGameMode->Spanwer->DebufferList.Find(Cast<ADebuffer>(Monster)) == INDEX_NONE)
							{
								TrapperGameMode->Spanwer->DebufferList.Add(Cast<ADebuffer>(Monster));
								GetWorld()->GetGameState<ATrapperGameState>()->ChangeMonsterCount(-1);
							}

							Cast<ADebuffer>(Monster)->bIsDebuffeStart = false;
							//Path Clear
							TObjectPtr<ADebufferMonsterAIController> DebufferAI = Cast<ADebufferMonsterAIController>(AIC);

							if (IsValid(DebufferAI))
							{
								for (int i = 0; i < DebufferAI->DebufferTargets.Num(); i++)
								{

									DebufferAI->DebufferTargets[i]->CheckTarget.Remove(Cast<ADebuffer>(Monster));
								}
							}
							//UE_LOG(LogTemp, Warning, TEXT("CDC Die"));
						}
						if (Cast<ABoss_Phase2>(Monster))
						{
							if (TrapperGameMode->Spanwer->BossPhase2List.Find(Cast<ABoss_Phase2>(Monster)) == INDEX_NONE)
							{
								TrapperGameMode->Spanwer->BossPhase2List.Add(Cast<ABoss_Phase2>(Monster));
								GetWorld()->GetGameState<ATrapperGameState>()->ChangeMonsterCount(-1);
							}
						
							//보스 죽은거 보냄
							if (Monster->HasAuthority())
							{
								GetWorld()->GetGameState<ATrapperGameState>()->OnQuestExecute.Broadcast(97, true);
							}
							//UE_LOG(LogTemp, Warning, TEXT("CDC Die"));
						}


						if (TrapperGameMode->Spanwer->SpawnMonsterList.Find(Monster) != INDEX_NONE)
						{
							TrapperGameMode->Spanwer->SpawnMonsterList.Remove(Monster);
							//GetWorld()->GetGameState<ATrapperGameState>()->ChangeMonsterCount(-1);
						}
					}

					
				}
				//Set MonsterAnimation As Idle
				Monster->ServerRPCIdle();
			}

			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
