// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class TRAPPERPROJECT_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void BeDrawnToPlayer();
	void SetCanBePulled(AActor* Player);
	bool HasPlayerRef();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<class UStaticMesh> FirstBoneMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<class UStaticMesh> SecondBoneMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UParticleSystemComponent* LightEffect;

private:
	TObjectPtr<class ATrapperPlayer> PlayerRef;

	FTimerHandle ItemDestroyHandle;
	float Acceleration = 10.f;
	float MaxAcceleration = 15.f;

	UFUNCTION() void AddBoneAndDestory();
};
