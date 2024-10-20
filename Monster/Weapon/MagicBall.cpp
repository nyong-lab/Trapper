// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Weapon/MagicBall.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "Engine/DamageEvents.h"
#include "NiagaraFunctionLibrary.h"

#include "Turret/Turret.h"

// Sets default values
AMagicBall::AMagicBall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MagicBall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagicBall"));
	RootComponent = MagicBall;

	MagicBallMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MagicBall Movement"));
	MagicBallMovement->MaxSpeed = 500.f;
	MagicBallMovement->InitialSpeed = 500.f;
	MagicBallMovement->bIsHomingProjectile = true;

}

// Called when the game starts or when spawned
void AMagicBall::BeginPlay()
{
	Super::BeginPlay();
	MagicBall->OnComponentHit.AddDynamic(this, &AMagicBall::OnHit);	
	MagicBall->SetCanEverAffectNavigation(false);
	//if (IsValid(TargetCharacter))
	//{
	//	MagicBallMovement->HomingTargetComponent = TargetCharacter->GetRootComponent();
	//	//UE_LOG(LogTemp, Warning, TEXT("MagicBall HomingCheck"));
	//}
}

// Called every frame
void AMagicBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsValid(TargetCharacter) && bIsPlayerDetected == true)
	{
		MagicBallMovement->HomingTargetComponent = TargetCharacter->GetRootComponent();
		//UE_LOG(LogTemp, Warning, TEXT("MagicBall HomingCheck"));
	}

	if (IsValid(TargetTurret) && bIsTurretDetected == true/* && !TargetTurret->IsPendingKill()*/)
	{
		MagicBallMovement->HomingTargetComponent = TargetTurret->GetRootComponent();
		//UE_LOG(LogTemp, Warning, TEXT("MagicBall Turret"));
	}

	if (IsValid(Tower) && bIsTowerDetected == true)
	{
		MagicBallMovement->HomingTargetComponent = Tower->GetRootComponent();
	}
	
	/*else
	{
		MagicBallMovement->bIsHomingProjectile = false;
		UE_LOG(LogTemp, Warning, TEXT("MagicBall Check"));
	}*/
}

void AMagicBall::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	
	//UE_LOG(LogTemp, Warning, TEXT("Hit Component : %s"), *OtherActor->GetName());
	
	if (!this->HasAuthority())
	{
		return;
	}

	auto MyOwner = GetOwner();
	//UE_LOG(LogTemp, Warning, TEXT("Owner Component : %s"), *MyOwner->GetName());

	if (!IsValid(MyOwner))
	{
		//Destroy();
		return;
	}

	//auto MyOwnerInstigator = MyOwner->GetInstigatorController();
	AController* MyOwnerInstigator = MyOwner->GetInstigatorController();
	auto DamageTypeClass = UDamageType::StaticClass();
	FDamageEvent DamageEvent;

	//UE_LOG(LogTemp, Warning, TEXT("Owner : %s"), *MyOwner->GetName());

	//OtherActor->TakeDamage(Damage, DamageEvent, nullptr, this);

	if (OtherActor != this && OtherActor != MyOwner && !OtherActor->ActorHasTag("Monster"))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Damage Check!"));

		UGameplayStatics::ApplyDamage(OtherActor, Damage, MyOwnerInstigator, this, DamageTypeClass);
		ServerRPCPlayHitEffect();
	}

	Destroy();
}

void AMagicBall::ServerRPCPlayHitEffect_Implementation()
{
	MulticastPRCPlayHitEffect();
	//UE_LOG(LogTemp, Warning, TEXT("Work?"));
}

void AMagicBall::MulticastPRCPlayHitEffect_Implementation()
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	if (HitParticle)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			HitParticle,
			MagicBall,
			NAME_None,
			FVector(0.f, 0.f, 100.f),
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true);
	}
}

