// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossThrowSword.generated.h"

UCLASS()
class TRAPPERPROJECT_API ABossThrowSword : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABossThrowSword();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	void DistoryActor() { Destroy(); }

private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Effect
	UFUNCTION(NetMulticast, Reliable)
	void ServerRPCPlayHitEffect();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPRCPlayHitEffect();

public:
	UPROPERTY(EditAnywhere, Category = "Sword")
	TObjectPtr<class UStaticMeshComponent> Sword;

	UPROPERTY(EditAnywhere, Category = "Sword")
	TObjectPtr<class UStaticMeshComponent> RootComp;

public:
	UPROPERTY(EditAnywhere)
	float Damage = 50.f;

	UPROPERTY(EditAnywhere)
	float SwordSpeed = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sword")
	TObjectPtr<class ACharacter> TargetCharacter;

	//Sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	TObjectPtr<USoundBase> HitSound;

	//Particle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	TObjectPtr<class UNiagaraSystem> HitParticle;

private:
	FTimerHandle TimerHandle;
	FTimerHandle TimerHandleDistory;

	FVector TargetCharacterLocation;
	uint8 bIsThrow : 1 = false;

	void CheckPlayerLocation();
};
