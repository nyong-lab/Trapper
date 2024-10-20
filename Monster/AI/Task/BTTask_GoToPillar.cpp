// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/AI/Task/BTTask_GoToPillar.h"
#include "Monster/BaseMonster.h"
#include "Monster/AI/BaseMonsterAIController.h"

EBTNodeResult::Type UBTTask_GoToPillar::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMenory)
{
	FVector ActorTolerance(0.f, 0.f, 20.f);
	TObjectPtr<class ABaseMonster> Monster = Cast<ABaseMonster>(OwnerComp.GetAIOwner()->GetPawn());
	
	if (IsValid(Monster))
	{
		FVector MonsterLocation = Monster->GetActorLocation();
		FVector PillarLocation = Monster->ActivedPillarLocation;
		FVector NewPosition = FMath::VInterpTo(MonsterLocation, PillarLocation, GetWorld()->GetDeltaSeconds(), 1.f);

		const float Tolerance = 100.f;
		
		if (MonsterLocation.Equals(PillarLocation, Tolerance))
		{
			Monster->MonsterState = EMonsterStateType::IsStun;
		}

		Monster->SetActorLocation(NewPosition);
	}

	return EBTNodeResult::Succeeded;
}
