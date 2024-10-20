// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrapSnabEnviromentBase.generated.h"

UCLASS()
class TRAPPERPROJECT_API ATrapSnabEnviromentBase : public AActor
{
	GENERATED_BODY()

public:
	ATrapSnabEnviromentBase();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	TArray<TObjectPtr<class UBoxComponent>> GetSnabTriggerBoxes() const { return SnapTriggerBoxes; }
	TObjectPtr<class UBoxComponent> GetSnapPoint() const { return SnapPoint; }

	FORCEINLINE bool IsInstalledTrap() const { return bInstalledTrap; }
	void SetInstalledTrap(bool Value);

	const class ATrapBase* FindChildTrap() const;

	void SetTrap(class ATrapBase* Trap);

	bool IsCanInstall() const;

protected:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// 기본 부착 컴포넌트들
	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class USceneComponent> DummyRoot;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UBoxComponent> SnapPoint;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TArray<TObjectPtr<class UBoxComponent>> SnapTriggerBoxes;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UBoxComponent> SnapTriggerrrrrrrrrrr;

	UPROPERTY(EditAnywhere, Category = "Trap")
	TObjectPtr<class UBoxComponent> CheckInstallTrigger;

	// 설치된 트랩
	UPROPERTY(ReplicatedUsing = OnRep_InstalledTrap)
	TObjectPtr<class ATrapBase> InstalledTrap;

	TSet<TObjectPtr<AActor>> OverlappingActors;

	/// 리플리케이션 -----------------------------------------------

	// 트랩이 설치되어 있는지 여부 (설치되어 있다면 중복 설치 못하게 막음)
	UPROPERTY(Replicated)
	uint8 bInstalledTrap : 1;

	uint8 bCanInstall : 1;

	/// OnRep -----------------------------------------------------
	UFUNCTION()
	void OnRep_InstalledTrap();
};
