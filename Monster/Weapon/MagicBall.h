// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicBall.generated.h"

UCLASS()
class TRAPPERPROJECT_API AMagicBall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagicBall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Effect
	UFUNCTION(NetMulticast, Reliable)
	void ServerRPCPlayHitEffect();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPRCPlayHitEffect();

private:
	UPROPERTY(EditAnywhere, Category = "MagicBall")
	TObjectPtr<class UStaticMeshComponent> MagicBall;

	UPROPERTY(EditAnywhere, Category = "MagicBall")
	TObjectPtr<class UProjectileMovementComponent> MagicBallMovement;

public:
	UPROPERTY(EditAnywhere)
	float Damage = 50.f;

	TObjectPtr<class ACharacter> TargetCharacter;
	TObjectPtr<class AActor> Tower;
	TObjectPtr<class ATurret> TargetTurret;

	uint8 bIsPlayerDetected : 1 = false;
	uint8 bIsTurretDetected : 1 = false;
	uint8 bIsTowerDetected  : 1 = false;

	//Sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	TObjectPtr<USoundBase> HitSound;

	//Particle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	TObjectPtr<class UNiagaraSystem> HitParticle;
};
