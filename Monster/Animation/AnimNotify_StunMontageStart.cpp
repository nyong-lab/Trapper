// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Animation/AnimNotify_StunMontageStart.h"
#include "Monster/BaseMonster.h"
#include "Monster/Debuffer.h"

void UAnimNotify_StunMontageStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (IsValid(MeshComp))
	{
		
		TObjectPtr<ABaseMonster>HitMonster = Cast<ABaseMonster>(MeshComp->GetOwner());
		TObjectPtr<ADebuffer> HitDebufferMonster = Cast<ADebuffer>(MeshComp->GetOwner());
		
		if (IsValid(HitMonster))
		{
			if (HitMonster->HasAuthority())
			{
				if (HitDebufferMonster)
				{
					HitDebufferMonster->MonsterState = EMonsterStateType::GoToTower;
				}

				else
				{
					HitMonster->ServerRPCStunLoop();
				}
			}
		}
	
	}
}
