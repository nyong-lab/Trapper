// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Weapon/BossThrowSword.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
ABossThrowSword::ABossThrowSword()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComp"));
	RootComponent = RootComp;

	Sword = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sword"));
	Sword->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void ABossThrowSword::BeginPlay()
{
	Super::BeginPlay();
	Sword->OnComponentHit.AddDynamic(this, &ABossThrowSword::OnHit);

	//Timer ¼³Á¤
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ABossThrowSword::CheckPlayerLocation, 1.5f, false);

}

void ABossThrowSword::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TargetCharacterLocation.Z -= 1;

	if (IsValid(TargetCharacter))
	{
		if (bIsThrow == false)
		{
			FVector PlayerLocation = TargetCharacter->GetActorLocation();
			FRotator LookAtRotation = FRotationMatrix::MakeFromX(PlayerLocation - GetActorLocation()).Rotator();

			SetActorRotation(LookAtRotation);
		}
		
		else
		{
			FVector CurrentLocation = GetActorLocation();
			FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetCharacterLocation, GetWorld()->GetDeltaSeconds(), SwordSpeed);
			SetActorLocation(NewLocation);
		}
				
	}

}

void ABossThrowSword::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Hit Component : %s"), *OtherActor->GetName());

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
		//Destroy();
	}

	//Destroy();
	GetWorld()->GetTimerManager().SetTimer(TimerHandleDistory, this, &ABossThrowSword::DistoryActor, 0.5f, false);
}

void ABossThrowSword::ServerRPCPlayHitEffect_Implementation()
{
	MulticastPRCPlayHitEffect();
}

void ABossThrowSword::MulticastPRCPlayHitEffect_Implementation()
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	if (HitParticle)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			HitParticle,
			Sword,
			NAME_None,
			FVector(0.f, 0.f, 0.f),
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true);
	}
}

void ABossThrowSword::CheckPlayerLocation()
{
	if (IsValid(TargetCharacter))
	{
		TargetCharacterLocation = TargetCharacter->GetActorLocation();
		bIsThrow = true;
		//UE_LOG(LogTemp, Warning, TEXT("Player Location : %f"), TargetCharacterLocation.X);
	}
}


