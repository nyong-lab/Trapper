// Fill out your copyright notice in the Description page of Project Settings.


#include "GameElement/Item.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Character/TrapperPlayer.h"

// Sets default values
AItem::AItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetVisibility(true);
	RootComponent = Mesh;

	LightEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Light Effect"));
	LightEffect->bAutoActivate = false;
	LightEffect->SetupAttachment(Mesh);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("/Script/Engine.ParticleSystem'/Game/Blueprints/VFX/Particles/P_ky_waterBall.P_ky_waterBall'"));
	if (ParticleAsset.Succeeded())
	{
		LightEffect->SetTemplate(ParticleAsset.Object);
	}
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(false);

	FMath::RandBool() ? Mesh->SetStaticMesh(FirstBoneMesh) : Mesh->SetStaticMesh(SecondBoneMesh);

	//UE_LOG(LogTemp, Warning, TEXT("Item Spawn!"));
}

void AItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	BeDrawnToPlayer();
}

void AItem::BeDrawnToPlayer()
{
	FVector ActorTolerance(0.f, 0.f, 20.f);

	FVector RightHandBoneLocation = PlayerRef->GetMesh()->GetBoneLocation(TEXT("hand_r"));
	FVector ItemLocation = GetActorLocation();
	FVector NewPosition = FMath::VInterpTo(ItemLocation, RightHandBoneLocation, GetWorld()->GetDeltaSeconds(), Acceleration);

	const float Tolerance = 15.f;
	if (ItemLocation.Equals(RightHandBoneLocation, Tolerance))
	{
		AddBoneAndDestory();
	}

	Acceleration = FMath::Min(Acceleration + (0.05f * GetWorld()->GetDeltaSeconds()), MaxAcceleration);
	SetActorLocation(NewPosition);
}

void AItem::SetCanBePulled(AActor* Player)
{
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(ItemDestroyHandle, this, &AItem::AddBoneAndDestory, 1.f, false, 1.5f);
	}

	PlayerRef = Cast<ATrapperPlayer>(Player);
	if (!PlayerRef) return;

	Mesh->SetVisibility(false);
	LightEffect->Activate();

	SetActorTickEnabled(true);
}

bool AItem::HasPlayerRef()
{
	return PlayerRef ? true : false;
}

void AItem::AddBoneAndDestory()
{
	if (PlayerRef->HasAuthority())
	{
		PlayerRef->AddBoneItem();
	}

	Destroy();
}

