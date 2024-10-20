// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap/Component/TrapMechanics.h"

#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Character/TrapperPlayer.h"
#include "Components/BoxComponent.h"
#include "Trap/TrapBase.h"
#include "Trap/TrabSnabEnviroment/TrapSnabEnviromentBase.h"
#include "Trap/TrabSnabEnviroment/TrapSnapTrigger.h"
#include "Net/UnrealNetwork.h"
#include "TrapperProject.h"
#include "NavigationSystem.h"
#include "Trap/ClapTrap.h"
#include "Trap/SpearTrap.h"
#include "Trap/GuillotinePendulumTrap.h"
#include "Framework/TrapperPlayerController.h"
#include "Widget/TrapSelectHUD.h"
#include "Widget/PlayerHUD.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/DecalActor.h"
#include "Framework/TrapperGameState.h"
#include "Widget/ScoreHUD.h"
#include "Widget/TrapSelectHUD.h"
#include "Components/TextBlock.h"

UTrapMechanics::UTrapMechanics()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// ������ ���̺��� �����Ͽ� Ʈ������ Array�� �����Ѵ�.
	/*static ConstructorHelpers::FObjectFinder<UDataTable> TrapPartsFinder(TEXT("/Script/Engine.DataTable'/Game/Trap/DataTable/DT_TrapParts.DT_TrapParts'"));
	if (TrapPartsFinder.Succeeded())
	{
		const UDataTable* DataTable = TrapPartsFinder.Object;
		check(DataTable->GetRowMap().Num() > 0);

		TArray<FName> RowNames;
		if (DataTable)
		{
			RowNames = DataTable->GetRowNames();

			for (const FName& RowName : RowNames)
			{
				FTrapParts* TrapPart = DataTable->FindRow<FTrapParts>(RowName, FString());
				if (TrapPart)
				{
					TrapPartsTable.Add(*TrapPart);
				}
			}
		}
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> TrapDataFinder(TEXT("/Script/Engine.DataTable'/Game/Blueprints/Data/DT_TrapData.DT_TrapData'"));
	if (TrapDataFinder.Succeeded())
	{
		const UDataTable* DataTable = TrapDataFinder.Object;
		check(DataTable->GetRowMap().Num() > 0);

		TArray<FName> RowNames;
		if (DataTable)
		{
			RowNames = DataTable->GetRowNames();

			for (const FName& RowName : RowNames)
			{
				FTrapInfo* TrapData = DataTable->FindRow<FTrapInfo>(RowName, FString());
				if (TrapData)
				{
					TrapDataTable.Add(*TrapData);
				}
			}
		}
	}*/

	LoadedBPGunpowderNiagaraClass = LoadObject<UClass>(nullptr, TEXT("/Game/Blueprints/Trap/TrapEffects/BP_GunpowderBarrelNiagara.BP_GunpowderBarrelNiagara_C"));

	static ConstructorHelpers::FObjectFinder<UInputAction> EnterInstallationModeActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_EnterInstallationMode.IA_EnterInstallationMode'"));
	if (EnterInstallationModeActionFinder.Succeeded())
	{
		EnterInstallationModeAction = EnterInstallationModeActionFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> ExitInstallationModeActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_EnterInstallationMode.IA_EnterInstallationMode'"));
	if (ExitInstallationModeActionFinder.Succeeded())
	{
		ExitInstallationModeAction = ExitInstallationModeActionFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> RealeseInstallationModeActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_RealeseInstallationMode.IA_RealeseInstallationMode'"));
	if (RealeseInstallationModeActionFinder.Succeeded())
	{
		RealeseInstallationModeAction = RealeseInstallationModeActionFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LeftMouseClickActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_LeftMouseClick.IA_LeftMouseClick'"));
	if (LeftMouseClickActionFinder.Succeeded())
	{
		LeftMouseClickAction = LeftMouseClickActionFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> RotatePreviewMeshActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_RotatePreviewMesh.IA_RotatePreviewMesh'"));
	if (RotatePreviewMeshActionFinder.Succeeded())
	{
		RotatePreviewMeshAction = RotatePreviewMeshActionFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> DemolishActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_Demolish.IA_Demolish'"));
	if (DemolishActionFinder.Succeeded())
	{
		DemolishAction = DemolishActionFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> PreviewMaterialBlueFinder(TEXT("/Script/Engine.Material'/Game/Trap/Material/M_PreviewMeshBlue.M_PreviewMeshBlue'"));
	if (PreviewMaterialBlueFinder.Succeeded())
	{
		PreviewMaterialBlue = PreviewMaterialBlueFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> PreviewMaterialRedFinder(TEXT("/Script/Engine.Material'/Game/Trap/Material/M_PreviewMeshRed.M_PreviewMeshRed'"));
	if (PreviewMaterialRedFinder.Succeeded())
	{
		PreviewMaterialRed = PreviewMaterialRedFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> SelectOverSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Sound/system_sounds/sfx_system_trap_select_over.sfx_system_trap_select_over'"));
	if (EnterInstallationModeActionFinder.Succeeded())
	{
		SelectOverSound = SelectOverSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> SelectDisableSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Sound/system_sounds/sfx_system_trap_select_disable.sfx_system_trap_select_disable'"));
	if (EnterInstallationModeActionFinder.Succeeded())
	{
		SelectDisableSound = SelectDisableSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> InstallFailSoundFinder(TEXT("/Script/Engine.SoundWave'/Game/Sound/trap_sounds/sfx_trap_fail.sfx_trap_fail'"));
	if (EnterInstallationModeActionFinder.Succeeded())
	{
		InstallFailSound = InstallFailSoundFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UClass> CheckClapPathFinder(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Trap/TrapEffects/BP_CheckClapPath.BP_CheckClapPath_C'"));
	if (CheckClapPathFinder.Succeeded())
	{
		CheckClapPath = CheckClapPathFinder.Object;
	}

	/*static ConstructorHelpers::FObjectFinder<UInputAction> ShowMeTheMoneyActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_ShowMeTheMoney.IA_ShowMeTheMoney'"));
	if (ShowMeTheMoneyActionFinder.Succeeded())
	{
		ShowMeTheMoneyAction = ShowMeTheMoneyActionFinder.Object;
	}*/

	static ConstructorHelpers::FObjectFinder<UInputAction> ShowScoreHUDActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_ShowScoreHUD.IA_ShowScoreHUD'"));
	if (ShowScoreHUDActionFinder.Succeeded())
	{
		ShowScoreHUDAction = ShowScoreHUDActionFinder.Object;
	}

	/*static ConstructorHelpers::FObjectFinder<UInputAction> UpdateScoreHUDActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_UpdateScoreHUD.IA_UpdateScoreHUD'"));
	if (UpdateScoreHUDActionFinder.Succeeded())
	{
		UpdateScoreHUDAction = UpdateScoreHUDActionFinder.Object;
	}*/

	static ConstructorHelpers::FObjectFinder<UInputAction> SpawnMonsterSpikeTrapsActionFinder(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/Actions/IA_SpawnMonsterSpikeTraps.IA_SpawnMonsterSpikeTraps'"));
	if (SpawnMonsterSpikeTrapsActionFinder.Succeeded())
	{
		SpawnMonsterSpikeTrapsAction = SpawnMonsterSpikeTrapsActionFinder.Object;
	}

	bInstallationMode = false;
	bSelectMode = false;
	bCanInstall = true;
	bCanSnap = false;
	bResetTrapPartsTableIndex = false;
	bAlignToSurface = false;
	bCanSelect = false;
	bCanRotationGuillotine = true;
	bBlockSelectTrap = false;
	bPlaceTrap = false;
	bPlayDemolishRaycast = false;
	bHitRayCanReInstall = false;
	bCanInstallCouple = false;
}

void UTrapMechanics::BeginPlay()
{
	Super::BeginPlay();


	if (TrapPartsDataTable)
	{
		TArray<FName> RowNames = TrapPartsDataTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			FTrapParts* TrapPart = TrapPartsDataTable->FindRow<FTrapParts>(RowName, FString());
			if (TrapPart)
			{
				TrapPartsArray.Add(*TrapPart);
			}
		}
	}

	if (TrapDataTable)
	{
		TArray<FName> RowNames = TrapDataTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			FTrapInfo* TrapPart = TrapDataTable->FindRow<FTrapInfo>(RowName, FString());
			if (TrapPart)
			{
				TrapDataArray.Add(*TrapPart);
			}
		}
	}

	Player = Cast<ATrapperPlayer>(GetOwner());
	if (IsValid(Player))
	{
		//PlayerController = Cast<ATrapperPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		PlayerController = Cast<ATrapperPlayerController>(Player->GetController());
		FollowCamera = Player->Camera;

		TrapperGameState = GetWorld()->GetGameState<ATrapperGameState>();
	}

	if (!IsValid(Player) || !Player->IsLocallyControlled())
	{
		return;
	}

	SetupInputComponent();

	/*GetWorld()->GetTimerManager().SetTimer(PreviewLoopTimerHandle, [this]()
		{
			if (bInstallationMode)
			{
				PreviewLoop();
			}
		}, 0.02, true);*/

	GetWorld()->GetTimerManager().SetTimer(PreviewLoopTimerHandle, this, &UTrapMechanics::PreviewLoop, 0.02f, true);


	//GetWorld()->GetTimerManager().SetTimer(DemolishReInstallRaycastLoopTimerHandle, [this]()
	//	{
	//		if (!bInstallationMode && !bSelectMode)
	//		{
	//			DemolishReInstallRaycastLoop();
	//		}
	//	}, 0.25, true);

	GetWorld()->GetTimerManager().SetTimer(DemolishReInstallRaycastLoopTimerHandle, this, &UTrapMechanics::DemolishReInstallRaycastLoop, 0.25f, true);
}

void UTrapMechanics::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void UTrapMechanics::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTrapMechanics, PreviewMesh);
	DOREPLIFETIME(UTrapMechanics, PreviewRange);
	DOREPLIFETIME(UTrapMechanics, PreviewCouple);
	DOREPLIFETIME(UTrapMechanics, PreviewNiagara);
	DOREPLIFETIME(UTrapMechanics, PreviewRangeDecal);
	DOREPLIFETIME(UTrapMechanics, TrapPartsTableIndex);
	DOREPLIFETIME(UTrapMechanics, PreviewMeshRotationYaw);
	DOREPLIFETIME(UTrapMechanics, TrapSnapTrigger);
	DOREPLIFETIME(UTrapMechanics, FocusTrap);
	DOREPLIFETIME(UTrapMechanics, PreviewFirstCoupleMeshTemp);
}

void UTrapMechanics::SetupInputComponent()
{
	AActor* OwnerActor = GetOwner();
	if (IsValid(OwnerActor))
	{
		//APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			UEnhancedInputComponent* UIC = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
			if (IsValid(UIC))
			{
				//UIC->BindAction(AcquireInstallationModeAction, ETriggerEvent::Started, this, &UTrapMechanics::EnterInstallationMode);
				UIC->BindAction(EnterInstallationModeAction, ETriggerEvent::Started, this, &UTrapMechanics::ReInstallTrap);
				UIC->BindAction(EnterInstallationModeAction, ETriggerEvent::Started, this, &UTrapMechanics::EnterSwitchSelectMode);
				UIC->BindAction(EnterInstallationModeAction, ETriggerEvent::Completed, this, &UTrapMechanics::ExitSwitchSelectMode);
				UIC->BindAction(RealeseInstallationModeAction, ETriggerEvent::Started, this, &UTrapMechanics::ReleaseInstallationMode);
				UIC->BindAction(LeftMouseClickAction, ETriggerEvent::Started, this, &UTrapMechanics::LeftMouseClick);
				UIC->BindAction(RotatePreviewMeshAction, ETriggerEvent::Triggered, this, &UTrapMechanics::RotatePreviewMesh);
				UIC->BindAction(DemolishAction, ETriggerEvent::Started, this, &UTrapMechanics::DemolishTrap);

				//UIC->BindAction(ShowMeTheMoneyAction, ETriggerEvent::Started, this, &UTrapMechanics::ShowMeTheMoney);
				UIC->BindAction(ShowScoreHUDAction, ETriggerEvent::Started, this, &UTrapMechanics::ShowScoreHUD);
				//UIC->BindAction(UpdateScoreHUDAction, ETriggerEvent::Started, this, &UTrapMechanics::UpdateScoreHUD);
				UIC->BindAction(SpawnMonsterSpikeTrapsAction, ETriggerEvent::Started, this, &UTrapMechanics::SpawnMonsterSpikeTraps);
			}
		}
	}
}

void UTrapMechanics::PreviewLoop()
{
	if (!IsValid(PreviewMesh) || !bInstallationMode)
	{
		return;
	}

	FTransform CameraWorldTransform = FollowCamera->GetComponentTransform();
	FVector CameraLocation = CameraWorldTransform.GetLocation();
	FQuat CameraRotation = CameraWorldTransform.GetRotation();
	FVector CameraForwardVector = CameraRotation.GetForwardVector();
	FVector StartLocation = CameraLocation + CameraForwardVector * FVector::Distance(Player->GetActorLocation(), CameraLocation);
	FVector EndLocation = StartLocation + CameraForwardVector * MaxInstallRayDistance;

	//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 5.f);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());
	CollisionParams.AddIgnoredComponent(PreviewMesh);

	// ���� ĳ����
	FHitResult HitResult;
	bool bHasHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, CollisionChannel, CollisionParams);
	bool bDetectSnabTrigger = false;

	if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spear)
	{
		bCanInstall = false;

		/*UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
		if (!NavSystem)
		{
			UE_LOG(LogTrap, Warning, TEXT("Navigation System is not valid."));
			return;
		}*/

		PreviewMeshRotator = FRotator(0.f, PreviewMeshRotationYaw, 0.f) + CameraRotation.Rotator();

		/*FVector MeshSize = PreviewMesh->GetStaticMesh()->GetBounds().BoxExtent;

		FVector ImpactPoint = HitResult.ImpactPoint;
		ImpactPoint.Z += MeshSize.Z * 2;*/

		FVector StartTrace;
		if (bHasHit)
		{
			//UE_LOG(LogTrap, Warning, TEXT("StartTrace ImpactPoint"));
			StartTrace = HitResult.ImpactPoint;
		}
		else
		{
			//UE_LOG(LogTrap, Warning, TEXT("StartTrace TraceEnd"));
			StartTrace = HitResult.TraceEnd;
		}

		//StartTrace.Z += 120.f;
		StartTrace.Z += 50.f;

		// PreviewMeshRotationYaw를 적용한 새로운 회전값 계산
		FRotator NewRotation = CameraRotation.Rotator();
		NewRotation.Yaw += (PreviewMeshRotationYaw - 90);

		// 새로운 회전값에서 RightVector를 얻기
		FVector RightVector = NewRotation.Quaternion().GetRightVector();
		//FVector RightVector = CameraRotation.GetRightVector();

		FVector SpearOffset = RightVector * 250;
		FVector SpearWallOffset = RightVector * 250;

		if (StartTrace.Z < Player->GetActorLocation().Z + 100)
		{
			PreviewMeshTransform.SetLocation(StartTrace - SpearOffset);
			PreviewCoupleTransform.SetLocation(StartTrace + SpearWallOffset);

			PreviewMeshTransform.SetRotation(FQuat(0.f, 0.f, PreviewMeshRotator.Quaternion().Z, PreviewMeshRotator.Quaternion().W));
			PreviewCoupleTransform.SetRotation(FQuat(0.f, 0.f, PreviewMeshRotator.Quaternion().Z, PreviewMeshRotator.Quaternion().W));
		}
		else
		{
			//UE_LOG(LogTrap, Warning, TEXT("RightVector : %s"), *RightVector.ToString());

			FVector EndTraceRight = StartTrace + (RightVector * SpearLeftRightRayDistance);
			FVector EndTraceLeft = StartTrace - (RightVector * SpearLeftRightRayDistance);

			FHitResult HitResultRight;
			FHitResult HitResultLeft;

			FCollisionQueryParams WallCollisionParams;
			//WallCollisionParams.bTraceComplex = true;
			/*WallCollisionParams.AddIgnoredComponent(PreviewMesh);
			WallCollisionParams.AddIgnoredComponent(PreviewCouple);*/

			/*FNavLocation NavLocationLeft;
			FNavLocation NavLocationRight;
			bool bIsNavmeshOnHitLeft = false;
			bool bIsNavmeshOnHitRight = false;*/

			//DrawDebugLine(GetWorld(), StartTrace, EndTraceLeft, FColor::Red);
			//DrawDebugLine(GetWorld(), StartTrace, EndTraceRight, FColor::Red);

			//bool bHasHitLeft = GetWorld()->LineTraceSingleByChannel(HitResultLeft, StartTrace, EndTraceLeft, ECC_WorldStatic, WallCollisionParams);
			// TrapRay
			bool bHasHitLeft = GetWorld()->LineTraceSingleByChannel(HitResultLeft, StartTrace, EndTraceLeft, ECC_GameTraceChannel8, WallCollisionParams);
			if (bHasHitLeft)
			{
				//UE_LOG(LogTrap, Warning, TEXT("bHasHitLeft"));
				//UE_LOG(LogTrap, Warning, TEXT("HitResultLeft Actor Name : %s"), *HitResultLeft.GetActor()->GetName());

				if (HitResultLeft.GetActor()->ActorHasTag("CannotInstallTrap"))
				{
					bHasHitLeft = false;
				}
				/*else
				{
					bIsNavmeshOnHitLeft = NavSystem->ProjectPointToNavigation(HitResultLeft.ImpactPoint, NavLocationLeft);
				}*/

				//UE_LOG(LogTrap, Warning, TEXT("NavLocationLeft : %s"), *NavLocationLeft.Location.ToString());
			}

			//bool bHasHitRight = GetWorld()->LineTraceSingleByChannel(HitResultRight, StartTrace, EndTraceRight, ECC_WorldStatic, WallCollisionParams);
			bool bHasHitRight = GetWorld()->LineTraceSingleByChannel(HitResultRight, StartTrace, EndTraceRight, ECC_GameTraceChannel8, WallCollisionParams);
			if (bHasHitRight)
			{
				//UE_LOG(LogTrap, Warning, TEXT("bHasHitRight"));
				//UE_LOG(LogTrap, Warning, TEXT("HitResultRight Actor Name : %s"), *HitResultRight.GetActor()->GetName());
				//UE_LOG(LogTrap, Warning, TEXT("HitResultRight GetComponent Name : %s"), *HitResultRight.GetComponent()->GetName());

				if (HitResultRight.GetActor()->ActorHasTag("CannotInstallTrap"))
				{
					bHasHitRight = false;
				}
				/*else
				{
					bIsNavmeshOnHitRight = NavSystem->ProjectPointToNavigation(HitResultRight.ImpactPoint, NavLocationRight);
				}*/

				//UE_LOG(LogTrap, Warning, TEXT("NavLocationRight : %s"), *NavLocationRight.Location.ToString());
			}

			if (bHasHitLeft && bHasHitRight/* && !bIsNavmeshOnHitLeft && !bIsNavmeshOnHitRight*/)
			{
				//UE_LOG(LogTrap, Warning, TEXT("Navi no"));
				//UE_LOG(LogTrap, Warning, TEXT("HitResultLeft Actor Name : %s"), *HitResultLeft.GetActor()->GetName());

				HitResultLeft.ImpactPoint.Z = StartTrace.Z;
				HitResultRight.ImpactPoint.Z = StartTrace.Z;

				/*FVector MeshPosition = HitResultLeft.ImpactPoint + (RightVector * MeshSize.Y);
				FVector CouplePosition = HitResultRight.ImpactPoint - (RightVector * MeshSize.Y);*/

				FVector MeshPosition = HitResultLeft.ImpactPoint + (RightVector * 25);
				//FVector CouplePosition = HitResultRight.ImpactPoint - (RightVector * 15);
				FVector CouplePosition = HitResultRight.ImpactPoint - (RightVector * 40);

				PreviewMeshTransform.SetLocation(MeshPosition);
				PreviewCoupleTransform.SetLocation(CouplePosition);

				PreviewMeshTransform.SetRotation(FQuat(0.f, 0.f, PreviewMeshRotator.Quaternion().Z, PreviewMeshRotator.Quaternion().W));
				PreviewCoupleTransform.SetRotation(FQuat(0.f, 0.f, PreviewMeshRotator.Quaternion().Z, PreviewMeshRotator.Quaternion().W));

				if (!CheckForOverlap())
				{
					// UE_LOG(LogTrap, Warning, TEXT("CheckForOverlap false"));

					/*if (PreviewMeshTransform.GetLocation().Z > Player->GetActorLocation().Z + 100)
					{
						UE_LOG(LogTrap, Warning, TEXT("PreviewMeshTransform.GetLocation().Z %f"), PreviewMeshTransform.GetLocation().Z);
						UE_LOG(LogTrap, Warning, TEXT("Player->GetActorLocation().Z %f"), Player->GetActorLocation().Z);
					}*/

					if (!CheckForOverlapSpearWall())
					{
						bCanInstall = true;
					}
				}

				/*UE_LOG(LogTrap, Warning, TEXT("HitResultLeft.ImpactPoint : %s"), *HitResultLeft.ImpactPoint.ToString());
				UE_LOG(LogTrap, Warning, TEXT("HitResultLeft.Actor Name : %s"), *HitResultLeft.GetActor()->GetName());
				UE_LOG(LogTrap, Warning, TEXT("HitResultRight.ImpactPoint : %s"), *HitResultRight.ImpactPoint.ToString());
				UE_LOG(LogTrap, Warning, TEXT("HitResultRight Actor Name : %s"), *HitResultRight.GetActor()->GetName());*/
			}
			else
			{
				//UE_LOG(LogTrap, Warning, TEXT("Navi yes"));

				PreviewMeshTransform.SetLocation(StartTrace - SpearOffset);
				PreviewCoupleTransform.SetLocation(StartTrace + SpearWallOffset);

				PreviewMeshTransform.SetRotation(FQuat(0.f, 0.f, PreviewMeshRotator.Quaternion().Z, PreviewMeshRotator.Quaternion().W));
				PreviewCoupleTransform.SetRotation(FQuat(0.f, 0.f, PreviewMeshRotator.Quaternion().Z, PreviewMeshRotator.Quaternion().W));
			}
		}

		goto SpearEnd;
	}

	// �浹 ���� 
	if (bHasHit)
	{
		//UE_LOG(LogTrap, Warning, TEXT("HitResult.GetActor()->GetName() : "), *HitResult.GetActor()->GetName());

		bCanInstall = true;

		/// õ�� ���� ���� (�⸧ �ָӴ�, ����)
		// Ư�� Ʈ���ſ� ������ ���� ���̸� �÷��ش�.
		if (TrapPartsArray[TrapPartsTableIndex].bIsCeling)
		{
			if (!CheckCellingTrap(HitResult, StartLocation, EndLocation, CameraForwardVector, CollisionParams, bHasHit))
			{
				//UE_LOG(LogTrap, Warning, TEXT("goto HasHitOilTriggerAndSecondHitFailed;"));
				goto HasHitOilTriggerAndSecondHitFailed;
			}
		}

		/// �����Ǵ� Ʈ��
		if (bCanSnap)
		{
			//CheckSnapTrap(HitResult, bDetectSnabTrigger, CameraRotation);

			if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GuillotinePendulum)
			{
				//UE_LOG(LogTrap, Warning, TEXT("bCanSnap GuillotinePendulum"));
				CheckSnabGuillotineTrap(HitResult, CameraRotation);
			}
		}

		/// (���� X)CheckOverlap 
		// �׻� ����Ʈ ����Ʈ�� ��ġ�� ����
		else
		{
			FVector TrapSpawnLocation = HitResult.ImpactPoint;
			PreviewMeshTransform.SetLocation(TrapSpawnLocation);
			//FRotator TrapRotator = FRotator(0.f, PreviewMeshRotationYaw, 0.f) + CameraRotation.Rotator();
			//PreviewMeshTransform.SetRotation(FQuat(0.f, 0.f, TrapRotator.Quaternion().Z, TrapRotator.Quaternion().W));
			//FRotator TrapRotator = PreviewMeshRotator + CameraRotation.Rotator();

			//PreviewMeshTransform.SetRotation(PreviewMeshRotator.Quaternion());

			// ���̿� ������� �ʾҴٸ� ���� ȯ�� ���� �ʱ�ȭ
			if (IsValid(TrapSnapEnviromentActor))
			{
				TrapSnapEnviromentActor = nullptr;
			}

			if (IsValid(TrapSnapTrigger))
			{
				TrapSnapTrigger = nullptr;
			}

			/// �⺻ ���� ������
			if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Plank)
			{
				PreviewMeshTransform.SetLocation(TrapSpawnLocation + FVector(0.f, 0.f, -270.f));
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::OilBag)
			{
				PreviewMeshTransform.SetLocation(TrapSpawnLocation + FVector(0.f, 0.f, -340.f));
			}

			if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spike)
			{
				PreviewMeshTransform.SetLocation(TrapSpawnLocation + FVector(0.f, 0.f, -25.f));
			}

			if (IsValid(PreviewMesh))
			{
				if (TrapPartsArray[TrapPartsTableIndex].bIsCeling || TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spear)
				{
					PreviewMeshRotator.Roll = 0;
					PreviewMeshRotator.Pitch = 0;

					if (CheckForOverlap())
					{
						bCanInstall = false;
					}
				}
				else
				{
					AlignToSurface();

					if (!bAlignToSurface)
					{
						PreviewMeshRotator.Roll = 0;
						PreviewMeshRotator.Pitch = 0;
					}


					if (CheckForOverlap())
					{
						bCanInstall = false;
					}
				}
			}

			// Ʈ�� ���� Ʈ�� ��ġ X
			if (bCanInstall && IsValid(HitResult.GetActor()) && HitResult.GetActor()->IsA(ATrapBase::StaticClass()))
			{
				bCanInstall = false;
			}

			if (bAlignToSurface)
			{
				//PreviewMeshRotator = PreviewMeshRotator + FRotator(0.f, PreviewMeshRotationYaw, 0.f);
				PreviewMeshTransform.SetRotation(PreviewMeshRotator.Quaternion());
			}
			else
			{
				//UE_LOG(LogTrap, Warning, TEXT("PreviewMeshRotationYaw : %f"), PreviewMeshRotationYaw);
				PreviewMeshRotator = FRotator(0.f, PreviewMeshRotationYaw, 0.f) + CameraRotation.Rotator();
				PreviewMeshTransform.SetRotation(FQuat(0.f, 0.f, PreviewMeshRotator.Quaternion().Z, PreviewMeshRotator.Quaternion().W));
			}
		}
	}
	/// �浹 ���� ����
	else
	{
		// ���� Ʈ���� �浹�� �� ��° ���� �浹 ���� ���� goto
	HasHitOilTriggerAndSecondHitFailed:

		if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GuillotinePendulum)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("392 if (TrapPartsTable[TrapPartsTableIndex].Type == ETrapType::GuillotinePendulum) True True"));

			if (!bCanRotationGuillotine)
			{
				bCanRotationGuillotine = true;

				if (TrapSnapTrigger && !TrapSnapTrigger->IsInstalledTrap())
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("400 if (TrapSnapTrigger && !TrapSnapTrigger->IsInstalledTrap()) True True"));

					if (Player->HasAuthority())
					{
						/*ATrapBase* SnapGuillotine = TrapSnapTrigger->GetSnapGuillotine();
						if (SnapGuillotine)
						{
							SnapGuillotine->SetActorHiddenInGame(true);
						}*/
						//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("409 MulticastRPCSnapGuillotineControl True"));
						MulticastRPCSnapGuillotineControl(TrapSnapTrigger->GetSnapGuillotine(), true, false);
						MulticastRPCSetHiddenPreviewMesh(false);
					}
					else
					{
						//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("415 ServerRPCSnapGuillotineControl True"));
						ServerRPCSnapGuillotineControl(TrapSnapTrigger->GetSnapGuillotine(), true, false);
						ServerRPCSetHiddenPreviewMesh(false);
					}

					TrapSnapTrigger = nullptr;
				}
			}
		}

		TrapSnapTrigger = nullptr;
		bCanInstall = false;
		bAlignToSurface = false;

		FVector TrapSpawnLocation = HitResult.TraceEnd;
		PreviewMeshTransform.SetLocation(TrapSpawnLocation);
		//FRotator TrapRotator = FRotator(0.f, PreviewMeshRotationYaw, 0.f) + CameraRotation.Rotator();
		//PreviewMeshTransform.SetRotation(FQuat(0.f, 0.f, TrapRotator.Quaternion().Z, TrapRotator.Quaternion().W));

		//FRotator TrapRotator = PreviewMeshRotator + CameraRotation.Rotator();
		//PreviewMeshTransform.SetRotation(TrapRotator.Quaternion());

		/*PreviewMeshRotator.Roll = 0;
		PreviewMeshRotator.Pitch = 0;
		PreviewMeshTransform.SetRotation(PreviewMeshRotator.Quaternion());*/

		PreviewMeshRotator = FRotator(0.f, PreviewMeshRotationYaw, 0.f) + CameraRotation.Rotator();
		PreviewMeshTransform.SetRotation(FQuat(0.f, 0.f, PreviewMeshRotator.Quaternion().Z, PreviewMeshRotator.Quaternion().W));
	}

	/*/// ���� ���� �⺻ ������ ����
	if (!bDetectSnabTrigger)
	{
		if (TrapPartsTable[TrapPartsTableIndex].Type == TrapType::GuillotinePendulum)
		{couple
			PreviewMeshTransform.SetLocation(PreviewMeshTransform.GetLocation() + GuillotinePendulumOffset);
		}
	}*/

	/*if (CheckForOverlap())
	{
		bCanInstall = false;
	}*/

	/// 박수 함정 설치 범위 체크
	if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap && bCanInstallCouple && bCanInstall && IsValid(PreviewFirstCoupleMeshTemp))
	{
		FVector FirstClapLocation = PreviewFirstCoupleMeshTemp->GetComponentLocation();
		FVector SecondClapLocation = PreviewMeshTransform.GetLocation();

		float Distance = abs(FVector::Distance(FirstClapLocation, SecondClapLocation));

		if (Distance < 750)
		{
			bCanInstall = false;

			/*UE_LOG(LogTrap, Warning, TEXT("Clap Install False"));
			UE_LOG(LogTrap, Warning, TEXT("Distance : %f"), Distance);
			UE_LOG(LogTrap, Warning, TEXT("ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ"));*/
		}
		else
		{
			FVector ExtentResult = PreviewMesh->GetStaticMesh()->GetBounds().BoxExtent;

			//UE_LOG(LogTrap, Warning, TEXT("ExtentResult : %s"), *ExtentResult.ToString());

			TArray<AActor*> ActorsToIgnore;

			FVector Direction = FirstClapLocation - SecondClapLocation;
			FVector NormalizedDirection = Direction.GetSafeNormal();

			SecondClapLocation.Z += ExtentResult.Z;
			float ClapDistance = abs(FVector::Distance(FirstClapLocation, SecondClapLocation));
			FVector ELocation = SecondClapLocation + (NormalizedDirection * ClapDistance);

			bHasHit = UKismetSystemLibrary::BoxTraceSingle(
				this,
				SecondClapLocation,
				ELocation,
				ExtentResult,
				PreviewMeshTransform.GetRotation().Rotator(),
				UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel13),
				false,
				ActorsToIgnore,
				//EDrawDebugTrace::ForOneFrame,
				EDrawDebugTrace::None,
				HitResult,
				true);

			//UE_LOG(LogTrap, Warning, TEXT("Clap Trace1"));

			if (bHasHit)
			{
				//UE_LOG(LogTrap, Warning, TEXT("Hit Name : %s"), *HitResult.GetActor()->GetName());
				bCanInstall = false;
			}
			else
			{
				ExtentResult.Z /= 2;
				SecondClapLocation.Z += ExtentResult.Z;
				ELocation.Z += ExtentResult.Z;
				//UE_LOG(LogTrap, Warning, TEXT("Clap Trace2"));

				bHasHit = UKismetSystemLibrary::BoxTraceSingle(
					this,
					SecondClapLocation,
					ELocation,
					ExtentResult,
					PreviewMeshTransform.GetRotation().Rotator(),
					UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel8),
					false,
					ActorsToIgnore,
					//EDrawDebugTrace::ForOneFrame,
					EDrawDebugTrace::None,
					HitResult,
					true);

				if (bHasHit)
				{
					bCanInstall = false;
				}
				else
				{
					//UE_LOG(LogTrap, Warning, TEXT("TrapRay Hit False"));
				}
			}
		}
	}

	/// ���� ���� ������ ������ Ʈ������ ����
	if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Plank ||
		TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::OilBag)
	{
		CalculateRangeMeshTransform();
	}

SpearEnd:
	// ������ �޽� ���� Ʈ������ �� ���׸���
	if (Player->HasAuthority())
	{
		AllPreviewSetTransform();
		MulticastRPCUpdatePreviewMeshMaterial(bCanInstall, TrapSnapTrigger);

		if (TrapPartsArray[TrapPartsTableIndex].bIsPreviewRange)
		{
			// ������ ������ ���� üũ
			if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GunpowderBarrel)
			{
				CheckServerBroadCastSetHiddenRange(true);
			}
		}
	}
	else
	{
		AllPreviewSetTransform();

		SafeNormalizeQuat();
		ServerRPCUpdatePreviewMeshTransform(PreviewMeshTransform, PreviewCoupleTransform, PreviewRangeTransform, PreviewMeshRotationYaw);
		ServerRPCUpdatePreviewMeshMaterial(bCanInstall, TrapSnapTrigger);

		if (TrapPartsArray[TrapPartsTableIndex].bIsPreviewRange)
		{
			// ������ ������ ���� üũ
			if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GunpowderBarrel)
			{
				CheckServerBroadCastSetHiddenRange(false);
			}
		}
	}
}

void UTrapMechanics::EnterInstallationMode()
{
	//HoveredTrapType
	if (Player->IsLocallyControlled() && !bInstallationMode)
	{
		if (Player->HasAuthority())
		{
			SetPreviewMesh();

			// Trap Anim
			MulticastRPCStartInstallAnim();
		}
		else
		{
			ServerRPCSetPreviewMesh(TrapPartsTableIndex);

			if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
			{
				PreviewMeshRotationYaw = 90;
			}

			// Trap Anim
			Player->StartInstallAnim();
			ServerRPCStartInstallAnim();
		}

		bInstallationMode = true;
		PlayerController->PlayerHudRef->SetTrapFrameVisibility(true);

		// �÷��̾�� ��ġ��� ���� �˸� (Ȱ ����)
		NotifyPlayerInstallationMode();

		// ���� ���� ���ο� �浹 ä�� ����
		// Ʈ���̽� ä���� �ݸ��� ä�η� ��ȯ�ؼ� �������� (���� �� �� ���)
		OnRep_TrapPartsTableIndex();

		//GetWorld()->GetTimerManager().SetTimer(PreviewLoopTimerHandle, this, &UTrapMechanics::PreviewLoop, 0.01f, true);
	}
}

void UTrapMechanics::SetPreviewMeshMaterial(bool bMaterialBlue, class ATrapSnapTrigger* TSTrigger)
{
	// ��ġ ���� ���ο� ���� ������ �޽� ���׸��� ���� ���� (���� or �ʷ�)
	if (IsValid(PreviewMesh))
	{
		if (bMaterialBlue)
		{
			//UE_LOG(LogTrap, Warning, TEXT("bMaterialBlue"));
			PreviewMesh->SetMaterial(0, PreviewMaterialBlue);

			if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spike)
			{
				PreviewMesh->SetMaterial(1, PreviewMaterialBlue);
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Plank)
			{
				PreviewMesh->SetMaterial(1, PreviewMaterialBlue);
				PreviewMesh->SetMaterial(2, PreviewMaterialBlue);
				PreviewMesh->SetMaterial(3, PreviewMaterialBlue);
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::OilBag)
			{
				PreviewMesh->SetMaterial(1, PreviewMaterialBlue);
				PreviewMesh->SetMaterial(2, PreviewMaterialBlue);
			}
			//else if (TrapPartsTable[TrapPartsTableIndex].Type == ETrapType::GuillotinePendulum)
			//{
			//	//UE_LOG(LogTrap, Warning, TEXT("SetMaterial"));

			//	if (IsValid(TSTrigger))
			//	{
			//		AGuillotinePendulumTrap* Guiilotion = Cast<AGuillotinePendulumTrap>(TSTrigger->GetSnapGuillotine());
			//		if (Guiilotion)
			//		{
			//			Guiilotion->AllMaterialBlue();
			//		}
			//	}
			//}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
			{
				if (TrapperGameState->IsCanPlaceClapTrap())
				{
					PreviewMesh->SetMaterial(1, PreviewMaterialBlue);
					PreviewMesh->SetMaterial(2, PreviewMaterialBlue);
					PreviewMesh->SetMaterial(3, PreviewMaterialBlue);
				}
				else
				{
					PreviewMesh->SetMaterial(1, PreviewMaterialRed);
					PreviewMesh->SetMaterial(2, PreviewMaterialRed);
					PreviewMesh->SetMaterial(3, PreviewMaterialRed);
				}
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Bear)
			{
				PreviewMesh->SetMaterial(1, PreviewMaterialBlue);
			}

			if (IsValid(PreviewCouple))
			{
				PreviewCouple->SetMaterial(0, PreviewMaterialBlue);

				if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
				{
					PreviewCouple->SetMaterial(1, PreviewMaterialBlue);
					PreviewCouple->SetMaterial(2, PreviewMaterialBlue);
					PreviewCouple->SetMaterial(3, PreviewMaterialBlue);
				}
			}

			if (IsValid(PreviewRange))
			{
				PreviewRange->SetMaterial(0, PreviewMaterialBlue);
			}
		}
		else
		{
			//UE_LOG(LogTrap, Warning, TEXT("bMaterialRed"));
			PreviewMesh->SetMaterial(0, PreviewMaterialRed);

			if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spike)
			{
				PreviewMesh->SetMaterial(1, PreviewMaterialRed);
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Plank)
			{
				PreviewMesh->SetMaterial(1, PreviewMaterialRed);
				PreviewMesh->SetMaterial(2, PreviewMaterialRed);
				PreviewMesh->SetMaterial(3, PreviewMaterialRed);
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::OilBag)
			{
				PreviewMesh->SetMaterial(1, PreviewMaterialRed);
				PreviewMesh->SetMaterial(2, PreviewMaterialRed);
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GuillotinePendulum)
			{
				PreviewMesh->SetMaterial(1, PreviewMaterialRed);
				PreviewMesh->SetMaterial(2, PreviewMaterialRed);
				PreviewMesh->SetMaterial(3, PreviewMaterialRed);
				PreviewMesh->SetMaterial(4, PreviewMaterialRed);
				PreviewMesh->SetMaterial(5, PreviewMaterialRed);
				PreviewMesh->SetMaterial(6, PreviewMaterialRed);

				if (IsValid(TSTrigger))
				{
					AGuillotinePendulumTrap* Guiilotion = Cast<AGuillotinePendulumTrap>(TSTrigger->GetSnapGuillotine());
					if (Guiilotion)
					{
						Guiilotion->AllMaterialRed();
					}
				}
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
			{
				PreviewMesh->SetMaterial(1, PreviewMaterialRed);
				PreviewMesh->SetMaterial(2, PreviewMaterialRed);
				PreviewMesh->SetMaterial(3, PreviewMaterialRed);
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Bear)
			{
				PreviewMesh->SetMaterial(1, PreviewMaterialRed);
			}

			if (IsValid(PreviewCouple))
			{
				PreviewCouple->SetMaterial(0, PreviewMaterialRed);

				if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
				{
					PreviewCouple->SetMaterial(1, PreviewMaterialRed);
					PreviewCouple->SetMaterial(2, PreviewMaterialRed);
					PreviewCouple->SetMaterial(3, PreviewMaterialRed);
				}
			}

			if (IsValid(PreviewRange))
			{
				PreviewRange->SetMaterial(0, PreviewMaterialRed);
			}
		}
	}
}

void UTrapMechanics::RotatePreviewMesh(const FInputActionValue& Value)
{
	/// ����, �ڼ� ������ �����̼� ���� x 
	if (Player->IsLocallyControlled())
	{
		if (!bCanRotationGuillotine)
		{
			if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GuillotinePendulum)
			{
				return;
			}
		}

		//UE_LOG(LogTrap, Warning, TEXT("RotatePreviewMesh"));

		/*if (TrapPartsTable[TrapPartsTableIndex].Type != ETrapType::OilBag)
		{
			PreviewMeshRotationYaw += Value.Get<float>() * 10;
		}*/

		PreviewMeshRotationYaw += Value.Get<float>() * 10;

		//// ������ Forward ���͸� ���� �������� ����
		//FVector LocalForward = PreviewMesh->GetForwardVector();

		//// ���� Forward ���͸� �������� ȸ�� ��Ʈ���� ����
		//FMatrix RotationMatrix = FRotationMatrix::MakeFromX(LocalForward);

		//// ȸ�� ��Ʈ������ FRotator�� ��ȯ
		//FRotator RotationFromMatrix = RotationMatrix.Rotator();

		//// Yaw ȸ�� ���� ����
		//RotationFromMatrix.Yaw += DeltaYaw;

		//// �ٽ� ȸ�� ��Ʈ������ ��ȯ
		////FMatrix NewRotationMatrix = FRotationMatrix::MakeFromX(RotationFromMatrix.Vector());

		//// PreviewMeshRotator�� ���ο� ȸ�� ���� �Ҵ�
		//PreviewMeshRotator = RotationFromMatrix;
	};
}

void UTrapMechanics::ReleaseInstallationMode()
{
	//UE_LOG(LogTrap, Warning, TEXT("ReleaseInstallationMode"));

	if (!Player->IsLocallyControlled())
	{
		return;
	}

	if (Player->IsPingSelectMode())
	{
		return;
	}

	if (bInstallationMode)
	{
		// Trap Anim
		if (Player->HasAuthority())
		{
			MulticastRPCEndInstallAnim();
		}
		else
		{
			Player->EndInstallAnim();
			ServerRPCEndInstallAnim();
		}


		bInstallationMode = false;
		bAlignToSurface = false;
		PlayerController->PlayerHudRef->SetTrapFrameVisibility(false);

		// �÷��̾�� ��ġ��� ���� �˸� (Ȱ ����)
		NotifyPlayerInstallationMode();

		//GetWorld()->GetTimerManager().ClearTimer(PreviewLoopTimerHandle);

		if (IsValid(PreviewMesh))
		{
			if (Player->HasAuthority())
			{
				MulticastRPCSafeReleasePreviewMesh();
				ReleasePreviewMesh();
			}
			else
			{
				SafeReleasePreviewMesh();
				ServerRPCRealesePreviewMesh(TrapSnapTrigger);
			}
		}

		bCanInstallCouple = false;

		if (!Player->HasAuthority() && !IsValid(PreviewMesh))
		{
			//UE_LOG(LogTrap, Warning, TEXT("	if (!Player->HasAuthority() && !IsValid(PreviewMesh)) true"));

			GetWorld()->GetTimerManager().SetTimer(ClientDestroyPreviweMeshTimerHandle, this, &UTrapMechanics::ClientDestroyPreviweMeshTimerCallback, 0.1, true);
		}

		if (!bPlaceTrap)
		{
			if (IsValid(TrapSnapTrigger))
			{
				ATrapBase* TrapBase = TrapSnapTrigger->GetSnapGuillotine();

				if (TrapBase)
				{
					if (Player->HasAuthority())
					{
						MulticastRPCSnapGuillotineControl(TrapBase, true, true);
					}
					else
					{
						ServerRPCSnapGuillotineControl(TrapBase, true, true);
					}
				}
			}
		}
	}
	else if (bSelectMode)
	{
		//UE_LOG(LogTrap, Warning, TEXT("ReleaseSelectMode"));
		ReleaseSelectMode();
		Player->ReleaseInstallationMode();
	}
}

void UTrapMechanics::ReleaseInstallationModeToPlace()
{
	if (bInstallationMode)
	{
		bInstallationMode = false;
		bAlignToSurface = false;
		PlayerController->PlayerHudRef->SetTrapFrameVisibility(false);

		// �÷��̾�� ��ġ��� ���� �˸� (Ȱ ����)
		NotifyPlayerInstallationMode();

		//GetWorld()->GetTimerManager().ClearTimer(PreviewLoopTimerHandle);

		if (IsValid(PreviewMesh))
		{
			if (IsValid(PreviewMesh))
			{
				PreviewMesh->SetHiddenInGame(true);
			}
			if (IsValid(PreviewRange))
			{
				PreviewRange->SetHiddenInGame(true);
			}
			if (IsValid(PreviewNiagara))
			{
				PreviewNiagara->SetActorHiddenInGame(true);
			}

			if (Player->HasAuthority())
			{
				ReleasePreviewMesh();
			}
			else
			{
				ServerRPCRealesePreviewMesh(TrapSnapTrigger);
			}
		}
	}
}

void UTrapMechanics::EnterSwitchSelectMode()
{
	if (!Player->IsLocallyControlled() || Player->IsPingSelectMode())
	{
		//UE_LOG(LogTrap, Warning, TEXT("	if (!Player->IsLocallyControlled() || Player->IsPingSelectMode())"));

		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("SwitchSelectMode"));

	if (IsValid(FocusTrap) && FocusTrap->IsWeckage())
	{
		//UE_LOG(LogTrap, Warning, TEXT("FocusTrap->IsWeckage() true"));
		return;
	}

	if (bHitRayCanReInstall)
	{
		//UE_LOG(LogTrap, Warning, TEXT("bDemolishReInstallMode"));
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("EnterSwitchSelectMode"));

	if (bBlockSelectTrap)
	{
		//UE_LOG(LogTrap, Warning, TEXT("bBlockSelectTrap"));
		return;
	}

	if (bInstallationMode)
	{
		//UE_LOG(LogTrap, Warning, TEXT("bInstallationMode"));
		//UE_LOG(LogTrap, Warning, TEXT("bInstallationMode"));
		ReleaseInstallationMode();
		return;
	}

	if (!bSelectMode)
	{
		//UE_LOG(LogTrap, Warning, TEXT("!bSelectMode"));
		EnterSelectMode();
		Player->EnterInstallationMode();
	}
	else if (bSelectMode)
	{
		//UE_LOG(LogTrap, Warning, TEXT("bSelectMode"));
		ReleaseSelectMode();
		Player->ReleaseInstallationMode();
	}
}

void UTrapMechanics::ExitSwitchSelectMode()
{
	if (!Player->IsLocallyControlled() || !bSelectMode)
	{
		return;
	}

	//if (!bCanSelect)
	//{
	//	//UE_LOG(LogTrap, Warning, TEXT("if (!bCanSelect)"));
	//	return;
	//}

	if (bHitRayCanReInstall)
	{
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("ExitSwitchSelectMode"));

	ReleaseSelectMode();

	if (HoveredTrapType == ETrapType::None)
	{
		//UE_LOG(LogTrap, Warning, TEXT("None"));

		if (bInstallationMode)
		{
			//UE_LOG(LogTrap, Warning, TEXT("ReleaseInstallationMode"));
			ReleaseInstallationMode();
		}

		Player->ReleaseInstallationMode();

		return;
	}

	// 박수 함정은 월드 내 하나만 존재해야 함
	if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
	{
		if (IsValid(TrapperGameState.Get()) && !TrapperGameState->IsCanPlaceClapTrap())
		{
			UE_LOG(LogTrap, Warning, TEXT("IsCanPlaceClapTrap false"));
			if (InstallFailSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, InstallFailSound, Player->GetActorLocation());
			}

			if (bInstallationMode)
			{
				ReleaseInstallationMode();
			}

			Player->ReleaseInstallationMode();
			return;
		}
	}
	else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spike)
	{
		if (!Player->IsInstallSpikeTrap())
		{
			if (InstallFailSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, InstallFailSound, Player->GetActorLocation());
			}

			if (bInstallationMode)
			{
				ReleaseInstallationMode();
			}

			Player->ReleaseInstallationMode();
			return;
		}
	}
	else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GunpowderBarrel)
	{
		if (!Player->IsInstallGunpowderBarrelTrap())
		{
			if (InstallFailSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, InstallFailSound, Player->GetActorLocation());
			}

			if (bInstallationMode)
			{
				ReleaseInstallationMode();
			}

			Player->ReleaseInstallationMode();
			return;
		}
	}

	if (Player->BoneItemBox >= ItemCost)
	{
		//UE_LOG(LogTrap, Warning, TEXT("Player->BoneItemBox >= ItemCost"));

		EnterInstallationMode();
	}
	else
	{
		//UE_LOG(LogTrap, Warning, TEXT("else"));
		Player->ReleaseInstallationMode();

		if (SelectDisableSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, SelectDisableSound, Player->GetActorLocation());
		}
	}
}

void UTrapMechanics::EnterSelectMode()
{
	if (!IsValid(PlayerController)
		|| !IsValid(PlayerController->TrapSelectHUDRef)
		|| !GEngine
		|| !GEngine->GameViewport
		|| !GEngine->GameViewport->Viewport
		)
	{
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("EnterSelectMode"));

	if (bInstallationMode)
	{
		//UE_LOG(LogTrap, Warning, TEXT("ReleaseInstallationMode"));
		ReleaseInstallationMode();
		Player->ReleaseInstallationMode();
	}

	bSelectMode = true;

	if (HoveredTrapType != ETrapType::None)
	{
		HoveredTrapType = ETrapType::None;
		PlayerController->TrapSelectHUDRef->OnHovered(ETrapType::None, ETrapHUDType::E_Enabled);
	}

	PlayerController->TrapSelectHUDRef->SetVisibility(ESlateVisibility::Visible);

	// 커서 관련
	Player->ShowCursor();
	Player->SetCursorToCenter();

	// 타이머
	GetWorld()->GetTimerManager().SetTimer(UpdateHoverCheckTimer, this, &UTrapMechanics::CheckOnHovered, 0.03, true);
}

void UTrapMechanics::ReleaseSelectMode()
{
	if (!IsValid(PlayerController) || !IsValid(PlayerController->TrapSelectHUDRef))
	{
		return;
	}

	bSelectMode = false;
	PlayerController->TrapSelectHUDRef->SetVisibility(ESlateVisibility::Collapsed);

	// 커서 
	Player->HideCursor();

	// 타이머
	GetWorld()->GetTimerManager().ClearTimer(UpdateHoverCheckTimer);
}


void UTrapMechanics::SetPreviewMesh()
{
	//UE_LOG(LogTrap, Warning, TEXT("SetPreviewMesh"));

	// ������ �޽� ���� 
	PreviewMesh = NewObject<UStaticMeshComponent>(GetOwner());
	//PreviewMesh = NewObject<UStaticMeshComponent>();

	PreviewMesh->SetIsReplicated(true);
	PreviewMesh->RegisterComponent();
	OnRep_PreviewMesh();

	// ������ �޽� üũ
	if (TrapPartsArray[TrapPartsTableIndex].bIsPreviewRange)
	{
		//UE_LOG(LogTrap, Warning, TEXT("bIsPreviewRange true"));
		//UE_LOG(LogTrap, Warning, TEXT("TrapPartsTableIndex : %d"), TrapPartsTableIndex);

		if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GunpowderBarrel)
		{
			FActorSpawnParameters SpawnParams;

			// ���� ����
			if (LoadedBPGunpowderNiagaraClass)
			{
				//UE_LOG(LogTrap, Warning, TEXT("LoadedBPGunpowderNiagaraClass"));

				/*FVector Location = PreviewMesh->GetComponentLocation();
				FRotator Rotation = FRotator::ZeroRotator;*/

				FTransform Transform = PreviewMesh->GetComponentTransform();

				// ���� ����
				//PreviewNiagara = GetWorld()->SpawnActor<AActor>(LoadedBPGunpowderNiagaraClass, Location, Rotation, SpawnParams);
				PreviewNiagara = GetWorld()->SpawnActor<AActor>(LoadedBPGunpowderNiagaraClass, Transform);
				OnRep_PreviewNiagara();

				//PreviewNiagara->SetActorEnableCollision(ECollisionEnabled::NoCollision);

				/*if (IsValid(PreviewNiagara))
				{
					UGameplayStatics::FinishSpawningActor(PreviewNiagara, Transform);
				}*/
			}
		}
		else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
		{
			if (bCanInstallCouple)
			{
				return;
			}

			//UE_LOG(LogTrap, Warning, TEXT("PreviewMeshRotationYaw = 90;"));
			PreviewMeshRotationYaw = 90;

			//UE_LOG(LogTrap, Warning, TEXT("Set PreviewRangeActor"));

			FTransform Transform = PreviewMesh->GetComponentTransform();
			//PreviewRangeDecal = GetWorld()->SpawnActorDeferred()<ADecalActor>(TrapPartsTable[TrapPartsTableIndex].PreviewRangeDecal, Transform);
			PreviewRangeDecal = GetWorld()->SpawnActorDeferred<ADecalActor>(TrapPartsArray[TrapPartsTableIndex].PreviewRangeDecal, Transform);

			PreviewRangeDecal->SetActorEnableCollision(ECollisionEnabled::NoCollision);
			PreviewRangeDecal->SetActorHiddenInGame(true);
			PreviewRangeDecal->SetActorScale3D(FVector(0.3, 1, 1));

			//OnRep_PreviewRangeDecal();

			//UE_LOG(LogTrap, Warning, TEXT("PreviewRangeActor Transform : %s"), *PreviewRangeActor->GetActorTransform().ToString());

			AActor* DecalActor = Cast<AActor>(PreviewRangeDecal);
			if (IsValid(DecalActor))
			{
				//UE_LOG(LogTrap, Warning, TEXT("PreviewRangeActor = true"));
				UGameplayStatics::FinishSpawningActor(DecalActor, Transform);
			}
		}
		else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spear)
		{
			PreviewMeshRotationYaw = 90;

			PreviewCouple = NewObject<UStaticMeshComponent>(GetOwner());
			PreviewCouple->SetIsReplicated(true);
			PreviewCouple->RegisterComponent();
			OnRep_PreviewCouple();
		}
		else
		{
			PreviewRange = NewObject<UStaticMeshComponent>(GetOwner());
			PreviewRange->SetIsReplicated(true);
			PreviewRange->AttachToComponent(PreviewMesh, FAttachmentTransformRules::KeepRelativeTransform);
			PreviewRange->RegisterComponent();
			OnRep_PreviewRange();
		}
	}
}

void UTrapMechanics::ReleasePreviewMesh()
{
	if (!IsValid(PreviewMesh))
	{
		return;
	}

	//UStaticMeshComponent* TempPreviewMesh = PreviewMesh;
	//UStaticMeshComponent* TempPreviewRange = PreviewRange;
	//UStaticMeshComponent* TempPreviewCouple = PreviewCouple;
	//AActor* TempPreviewNiagara = PreviewNiagara;

	//PreviewMeshTransform.SetIdentity();
	//PreviewMeshRotator = FRotator(0, 0, 0);
	//PreviewMeshRotationYaw = 0.f;

	//PreviewMesh = nullptr;
	//PreviewRange = nullptr;
	//PreviewNiagara = nullptr;

	//FTimerHandle DestroyHandle;

	//if (bCanInstallCouple)
	//{
	//	UStaticMeshComponent* TempFirstPreviewCouple = PreviewFirstCoupleMeshTemp;
	//	ADecalActor* TempPreviewRangeDecal = PreviewRangeDecal;

	//	PreviewFirstCoupleMeshTemp = nullptr;
	//	PreviewRangeDecal = nullptr;

	//	GetWorld()->GetTimerManager().SetTimer(DestroyHandle, [this, TempPreviewMesh, TempPreviewRange, TempPreviewNiagara, TempFirstPreviewCouple, TempPreviewRangeDecal]()
	//		{
	//			//UE_LOG(LogTrap, Warning, TEXT("bCanInstallCouple Timer"));

	//			if (IsValid(TempPreviewMesh))
	//			{
	//				TempPreviewMesh->SetStaticMesh(nullptr);
	//				TempPreviewMesh->UnregisterComponent();
	//				TempPreviewMesh->DestroyComponent();
	//			}

	//			if (IsValid(TempPreviewRange))
	//			{
	//				TempPreviewRange->SetStaticMesh(nullptr);
	//				TempPreviewRange->UnregisterComponent();
	//				TempPreviewRange->DestroyComponent();
	//			}

	//			if (IsValid(TempPreviewNiagara))
	//			{
	//				TempPreviewNiagara->Destroy();
	//			}

	//			if (IsValid(TempFirstPreviewCouple))
	//			{
	//				//UE_LOG(LogTrap, Warning, TEXT("TempFirstPreviewCouple True"));
	//				TempFirstPreviewCouple->SetStaticMesh(nullptr);
	//				TempFirstPreviewCouple->UnregisterComponent();
	//				TempFirstPreviewCouple->DestroyComponent();
	//			}
	//			else
	//			{
	//				//UE_LOG(LogTrap, Warning, TEXT("TempFirstPreviewCouple False"));
	//			}

	//			if (IsValid(TempPreviewRangeDecal))
	//			{
	//				TempPreviewRangeDecal->Destroy();
	//			}

	//		}, 3.f, false);

	//	bCanInstallCouple = false;
	//}
	//else
	//{
	//	GetWorld()->GetTimerManager().SetTimer(DestroyHandle, [this, TempPreviewMesh, TempPreviewRange, TempPreviewCouple, TempPreviewNiagara]()
	//		{
	//			if (IsValid(TempPreviewMesh))
	//			{
	//				TempPreviewMesh->SetStaticMesh(nullptr);
	//				TempPreviewMesh->UnregisterComponent();
	//				TempPreviewMesh->DestroyComponent();
	//			}

	//			if (IsValid(TempPreviewRange))
	//			{
	//				TempPreviewRange->SetStaticMesh(nullptr);
	//				TempPreviewRange->UnregisterComponent();
	//				TempPreviewRange->DestroyComponent();
	//			}

	//			if (IsValid(TempPreviewCouple))
	//			{
	//				TempPreviewCouple->SetStaticMesh(nullptr);
	//				TempPreviewCouple->UnregisterComponent();
	//				TempPreviewCouple->DestroyComponent();
	//			}

	//			if (IsValid(TempPreviewNiagara))
	//			{
	//				TempPreviewNiagara->Destroy();
	//			}
	//		}, 3.f, false);
	//}

	PreviewMeshTransform.SetIdentity();
	PreviewMeshRotator = FRotator(0, 0, 0);
	PreviewMeshRotationYaw = 0.f;


	if (bCanInstallCouple)
	{
		if (IsValid(PreviewMesh))
		{
			PreviewMesh->SetStaticMesh(nullptr);
			PreviewMesh->UnregisterComponent();
			PreviewMesh->DestroyComponent();
		}

		if (IsValid(PreviewRange))
		{
			PreviewRange->SetStaticMesh(nullptr);
			PreviewRange->UnregisterComponent();
			PreviewRange->DestroyComponent();
		}

		if (IsValid(PreviewNiagara))
		{
			PreviewNiagara->Destroy();
		}

		if (IsValid(PreviewFirstCoupleMeshTemp))
		{
			//UE_LOG(LogTrap, Warning, TEXT("TempFirstPreviewCouple True"));
			PreviewFirstCoupleMeshTemp->SetStaticMesh(nullptr);
			PreviewFirstCoupleMeshTemp->UnregisterComponent();
			PreviewFirstCoupleMeshTemp->DestroyComponent();
		}
		else
		{
			//UE_LOG(LogTrap, Warning, TEXT("TempFirstPreviewCouple False"));
		}

		if (IsValid(PreviewRangeDecal))
		{
			PreviewRangeDecal->Destroy();
		}

		bCanInstallCouple = false;
	}
	else
	{
		if (IsValid(PreviewMesh))
		{
			PreviewMesh->SetStaticMesh(nullptr);
			PreviewMesh->UnregisterComponent();
			PreviewMesh->DestroyComponent();
		}

		if (IsValid(PreviewRange))
		{
			PreviewRange->SetStaticMesh(nullptr);
			PreviewRange->UnregisterComponent();
			PreviewRange->DestroyComponent();
		}

		if (IsValid(PreviewCouple))
		{
			PreviewCouple->SetStaticMesh(nullptr);
			PreviewCouple->UnregisterComponent();
			PreviewCouple->DestroyComponent();
		}

		if (IsValid(PreviewNiagara))
		{
			PreviewNiagara->Destroy();
		}
	}

	PreviewMesh = nullptr;
	PreviewRange = nullptr;
	PreviewNiagara = nullptr;
	PreviewFirstCoupleMeshTemp = nullptr;
	PreviewRangeDecal = nullptr;
}

void UTrapMechanics::LeftMouseClick()
{
	//UE_LOG(LogTrap, Warning, TEXT("LeftMouseClick"));

	if (Player && !Player->IsLocallyControlled())
	{
		return;
	}

	if (Player->IsPingSelectMode())
	{
		return;
	}

	if (bInstallationMode)
	{
		if (bCanInstall)
		{
			PlaceTrap();
		}
		else
		{
			if (InstallFailSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, InstallFailSound, Player->GetActorLocation());
			}
		}
	}

	/*if (bSelectMode)
	{
		if (!bCanSelect)
		{
			UE_LOG(LogTrap, Warning, TEXT("if (!bCanSelect)"));
			return;
		}

		UE_LOG(LogTrap, Warning, TEXT("LeftMouseClick : bSelectMode"));
		if (Player->BoneItemBox >= ItemCost)
		{
			UE_LOG(LogTrap, Warning, TEXT("Player->BoneItemBox >= ItemCost"));

			EnterInstallationMode();
			ReleaseSelectMode();
		}
	}*/
}

void UTrapMechanics::PlaceTrap()
{
	//UE_LOG(LogTrap, Warning, TEXT("PlaceTrap"));

	if (TrapPartsArray.IsValidIndex(TrapPartsTableIndex))
	{
		//UE_LOG(LogTrap, Warning, TEXT("PlaceTrap"));
		//UE_LOG(LogTrap, Warning, TEXT("Player->UseBoneItem(ItemCost)"));
		//UE_LOG(LogTrap, Warning, TEXT("ItemCost : %d"), ItemCost);

		if (Player->BoneItemBox < ItemCost)
		{
			return;
		}

		if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
		{
			//UE_LOG(LogTrap, Warning, TEXT("bIsCouple true"));

			if (!TrapperGameState->IsCanPlaceClapTrap())
			{
				if (InstallFailSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, InstallFailSound, Player->GetActorLocation());
				}
				return;
			}

			if (!bCanInstallCouple)
			{
				//UE_LOG(LogTrap, Warning, TEXT("bCanInstallCouple false"));
				bCanInstallCouple = true;

				if (IsValid(PreviewRangeDecal))
				{
					//UE_LOG(LogTrap, Warning, TEXT("PreviewRangeDecal true"));

					if (Player->HasAuthority())
					{
						//UE_LOG(LogTrap, Warning, TEXT("Server PlaceTrap"));

						PreviewFirstCoupleMeshTemp = PreviewMesh;
						PreviewMesh = nullptr;
						PreviewRangeDecal->SetActorHiddenInGame(false);

						//MulticastRPCFirstCoupleInstall(PreviewRangeDecal);
						SetPreviewMesh();
					}
					else
					{
						//UE_LOG(LogTrap, Warning, TEXT("Client PlaceTrap"));

						PreviewMesh = nullptr;
						ServerRPCFirstCoupleInstall();
					}

					return;
				}
			}
		}

		Player->UseBoneItem(ItemCost);

		if (Player->HasAuthority())
		{
			bool bInstalledTrap = PermissionPlaceTrap();
			if (bInstalledTrap)
			{
				PlaySoundInstallTrap();
			}
		}
		else
		{
			ServerRPCPlaceTrap(TrapSnapEnviromentActor, TrapSnapTrigger);
		}


	}

	bPlaceTrap = true;

	ReleaseInstallationMode();

	bPlaceTrap = false;
}

bool UTrapMechanics::PermissionPlaceTrap()
{
	bool bSpawnedTrap = false;

	if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GuillotinePendulum)
	{
		if (IsValid(TrapSnapTrigger))
		{
			MulticastRPCGuillotineSpawn(TrapSnapTrigger);

			ATrapBase* SnapGuillotine = TrapSnapTrigger->GetSnapGuillotine();

			InitializeTrap(SnapGuillotine);

			if (IsValid(TrapSnapTrigger))
			{
				TrapSnapTrigger->SetTrap(SnapGuillotine);
				TrapSnapTrigger = nullptr;
			}

			bSpawnedTrap = true;
		}
	}
	else
	{
		TSubclassOf<ATrapBase> TrapActorClass;
		TrapActorClass = TrapPartsArray[TrapPartsTableIndex].ClassType;
		if (TrapActorClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Player;

			ATrapBase* SpawnedTrap = GetWorld()->SpawnActorDeferred<ATrapBase>(TrapActorClass, PreviewMeshTransform, Player);
			if (IsValid(SpawnedTrap))
			{
				bSpawnedTrap = true;

				InitializeTrap(SpawnedTrap);

				/*if (TrapPartsTable[TrapPartsTableIndex].Type != TrapType::GuillotinePendulum)
				{
					FRotator TestRotator = FRotator(0, PreviewMeshRotationYaw, 0);
					SpawnedTrap->GetMesh()->AddLocalRotation(TestRotator);
				}*/

				UGameplayStatics::FinishSpawningActor(SpawnedTrap, PreviewMeshTransform);
			}

			if (bCanInstallCouple)
			{
				if (!PreviewFirstCoupleMeshTemp)
				{
					//UE_LOG(LogTrap, Warning, TEXT("PreviewFirstCoupleMeshTemp nullptr"));
					return true;
				}

				FTransform FirstCoupleTransform = PreviewFirstCoupleMeshTemp->GetComponentTransform();

				ATrapBase* SpawnedFirstCouple = GetWorld()->SpawnActorDeferred<ATrapBase>(TrapActorClass, FirstCoupleTransform, Player);
				if (IsValid(SpawnedFirstCouple))
				{
					InitializeTrap(SpawnedFirstCouple);

					UGameplayStatics::FinishSpawningActor(SpawnedFirstCouple, FirstCoupleTransform);

					AClapTrap* Clap = Cast<AClapTrap>(SpawnedTrap);
					if (IsValid(Clap))
					{
						Clap->SetOtherClapInteraction(SpawnedFirstCouple);
					}

					/*for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
					{
						APlayerController* PC = It->Get();
						if (PC && PC != PlayerController)
						{
							UTrapMechanics* TrapMechanics = PC->FindComponentByClass<UTrapMechanics>();
							if (TrapMechanics)
							{
								TrapMechanics->ClientRPCCheckClapAndRealeseInstallationMode();
							}
						}
					}*/
				}

				FTransform ClapPathTransform = FTransform::Identity;
				AActor* SpawnedClapPath = GetWorld()->SpawnActorDeferred<AActor>(CheckClapPath, ClapPathTransform, SpawnedFirstCouple);
				if (IsValid(SpawnedClapPath))
				{
					//UE_LOG(LogTrap, Warning, TEXT("SpawnedClapPath")   );
					UGameplayStatics::FinishSpawningActor(SpawnedClapPath, ClapPathTransform);

					SpawnedClapPath->AttachToActor(SpawnedFirstCouple, FAttachmentTransformRules::KeepRelativeTransform);


					FVector LocationA = PreviewMeshTransform.GetLocation();
					FVector LocationB = FirstCoupleTransform.GetLocation();

					FVector Direction = LocationB - LocationA;
					FVector NormalizedDirection = Direction.GetSafeNormal();

					float Distance = Direction.Size();

					// 회전
					FRotator NewRotation = NormalizedDirection.Rotation();
					SpawnedClapPath->SetActorRotation(NewRotation);

					// 위치
					FVector CentralPosition = (LocationA + LocationB) / 2.0f;
					SpawnedClapPath->SetActorLocation(CentralPosition);

					// 스케일
					FVector DefaultScale = SpawnedClapPath->GetActorScale3D();
					//float DefaultLength = SpawnedClapPath->FindComponentByClass<UStaticMeshComponent>()->GetStaticMesh()->GetBounds().BoxExtent.X;
					float DefaultLength = PreviewFirstCoupleMeshTemp->GetStaticMesh()->GetBounds().BoxExtent.X;
					DefaultLength *= 2;
					float NewScaleX = Distance / DefaultLength;
					FVector NewScale = FVector(NewScaleX, DefaultScale.Y, DefaultScale.Z);

					SpawnedClapPath->SetActorScale3D(NewScale);
				}

				if (IsValid(TrapperGameState.Get()))
				{
					//UE_LOG(LogTrap, Warning, TEXT("GameState ClapTrap false"));
					TrapperGameState->NextCanPlaceClapTrap(false);
				}
			}

			if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spear)
			{
				UStaticMeshComponent* SpearWall = SpawnedTrap->FindComponentByTag<UStaticMeshComponent>("SpearWall");
				if (IsValid(SpearWall))
				{
					SpearWall->SetWorldTransform(PreviewCoupleTransform);

					ASpearTrap* SpearTrap = Cast<ASpearTrap>(SpawnedTrap);
					if (IsValid(SpearTrap))
					{
						SpearTrap->SetMapMarkerSpearWallInitialLocation();
						SpearTrap->ConnectionSpearAndWall();
					}
				}
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GunpowderBarrel)
			{
				if (IsValid(PreviewNiagara))
				{
					PreviewNiagara->Destroy();
				}

				IncreaseTrapHudNum(ETrapType::GunpowderBarrel, true);
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spike)
			{
				IncreaseTrapHudNum(ETrapType::Spike, true);
			}

			// ���� ȯ�� ���Ͱ� ����Ǿ� �ִٸ� Ʈ�� ��ġ ���� üũ
			if (IsValid(TrapSnapEnviromentActor))
			{
				TrapSnapEnviromentActor->SetTrap(SpawnedTrap);
				TrapSnapEnviromentActor = nullptr;
			}
		}
	}

	if (bSpawnedTrap)
	{
		if (IsValid(Player) && TrapperGameState.Get())
		{
			if (Player->Is1Player())
			{
				TrapperGameState->AddScore(EScoreType::TrapScore1P, 10);
			}
			else
			{
				TrapperGameState->AddScore(EScoreType::TrapScore2P, 10);
			}
		}
	}

	return bSpawnedTrap;
}

//void UTrapMechanics::ChangePreviewMesh()
//{
//	if (Player->IsLocallyControlled() && bInstallationMode)
//	{
//		if (Player->HasAuthority())
//		{
//			PermissionChangePreviewMesh();
//			OnRep_TrapPartsTableIndex();
//		}
//
//		else
//		{
//			ServerRPCChangePreviewMesh();
//		}
//	}
//}
//
//void UTrapMechanics::PermissionChangePreviewMesh()
//{
//	if (!bResetTrapPartsTableIndex)
//	{
//		TrapPartsTableIndex = FMath::Clamp(TrapPartsTableIndex + 1, 0, TrapPartsTable.Num() - 1);
//
//		if (TrapPartsTableIndex == TrapPartsTable.Num() - 1)
//		{
//			bResetTrapPartsTableIndex = true;
//		}
//	}
//	else
//	{
//		TrapPartsTableIndex = 0;
//		bResetTrapPartsTableIndex = false;
//	}
//
//	RealesePreviewMesh();
//	SetPreviewMesh();
//}

ECollisionChannel UTrapMechanics::ConvertTraceTypeToCollisionChannel(ETraceTypeQuery TraceChannel)
{
	const int32 Index = static_cast<int32>(TraceChannel) /*- 1*/;
	//UE_LOG(LogTrap, Warning, TEXT("TraceChannel Index: %d"), Index);

	switch (Index)
	{
	case 0:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel1"));
		return ECollisionChannel::ECC_GameTraceChannel1;

	case 1:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel2"));
		return ECollisionChannel::ECC_GameTraceChannel2;

	case 2:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel3"));
		return ECollisionChannel::ECC_GameTraceChannel3;

	case 3:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel4"));
		return ECollisionChannel::ECC_GameTraceChannel4;

	case 4:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel5"));
		return ECollisionChannel::ECC_GameTraceChannel5;

	case 5:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel6"));
		return ECollisionChannel::ECC_GameTraceChannel6;

	case 6:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel7"));
		return ECollisionChannel::ECC_GameTraceChannel7;

	case 7:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel8"));
		return ECollisionChannel::ECC_GameTraceChannel8;

	case 8:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel9"));
		return ECollisionChannel::ECC_GameTraceChannel9;

	case 9:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel10"));
		return ECollisionChannel::ECC_GameTraceChannel10;

	case 10:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel11"));
		return ECollisionChannel::ECC_GameTraceChannel11;

	case 11:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel12"));
		return ECollisionChannel::ECC_GameTraceChannel12;

	case 12:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel13"));
		return ECollisionChannel::ECC_GameTraceChannel13;

	case 13:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel14"));
		return ECollisionChannel::ECC_GameTraceChannel14;

	case 14:
		//UE_LOG(LogTemp, Warning, TEXT("ECC_GameTraceChannel15"));
		return ECollisionChannel::ECC_GameTraceChannel15;

	default:
		//UE_LOG(LogTemp, Warning, TEXT("TrapMechanics RayCastring Collision Channel = default : ECC_Visibility"));
		return ECollisionChannel::ECC_Visibility;
	}
}

bool UTrapMechanics::DetectSnapTriggerBoxes(const FHitResult& HitResult)
{
	// ���Ͱ� ATrapSnabEnviromentActor Ŭ�������� Ȯ��
	ATrapSnabEnviromentBase* SnabActor = Cast<ATrapSnabEnviromentBase>(HitResult.GetActor());
	if (IsValid(SnabActor))
	{
		// �ش� Ŭ������ Ʈ���� �ڽ����� ��ȸ�ϸ� ����� ������Ʈ�� ��ġ�ϴٸ�
		// ���� ����Ʈ�� ������ �޽� Ʈ������ ����
		for (const auto& TriggerBox : SnabActor->GetSnabTriggerBoxes())
		{
			if (IsValid(TriggerBox))
			{
				if (TriggerBox == HitResult.GetComponent())
				{
					/*PreviewMeshTransform = SnabActor->GetSnapPoint()->GetComponentTransform();
					PreviewMeshTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));*/
					return true;
				}
			}
		}
	}

	return false;
}

bool UTrapMechanics::DetectSnapTriggerBox(const FHitResult& HitResult)
{
	UBoxComponent* TriggerBox = TrapSnapTrigger->GetSnabTriggerBox();

	//UE_LOG(LogTrap, Warning, TEXT("TriggerBox Name : %s"), *TriggerBox->GetName());
	//UE_LOG(LogTrap, Warning, TEXT("HitResult.GetComponent() Name : %s"), *HitResult.GetComponent()->GetName());

	if (IsValid(TriggerBox))
	{
		if (TriggerBox == HitResult.GetComponent())
		{
			//UE_LOG(LogTrap, Warning, TEXT("TriggerBox == HitResult.GetComponent()"));

			PreviewMeshTransform = TrapSnapTrigger->GetSnapPoint()->GetComponentTransform();
			//PreviewMeshTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
			return true;
		}
	}

	return false;
}
void UTrapMechanics::CheckSnabGuillotineTrap(FHitResult& HitResult, FQuat& CameraRotation)
{
	AActor* HitResultActor = HitResult.GetActor();
	bool bDetectSnabTrigger = false;

	//UE_LOG(LogTrap, Warning, TEXT("CheckSnabGuillotineTrap"));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CheckSnabGuillotineTrap"));

	// Ʈ�� ������ ������ ȯ�� ������Ʈ���� üũ 
	if (HitResultActor->IsA(ATrapSnapTrigger::StaticClass()))
	{
		//UE_LOG(LogTrap, Warning, TEXT("IsA(ATrapSnapTrigger)"));
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("HitResultActor->IsA(ATrapSnapTrigger::StaticClass()) True True"));

		TrapSnapTrigger = Cast<ATrapSnapTrigger>(HitResultActor);

		// ���Ϳ� Ʈ���� �̹� ��ġ�Ǿ� �ִ��� üũ (�ߺ� ��ġ x)
		if (IsValid(TrapSnapTrigger) && TrapSnapTrigger->IsInstalledTrap())
		{
			//UE_LOG(LogTrap, Warning, TEXT("if (IsValid(TrapSnapTrigger) && TrapSnapTrigger->IsInstalledTrap())"));
			bDetectSnabTrigger = false;
			TrapSnapTrigger = nullptr;
		}
		else
		{
			//UE_LOG(LogTrap, Warning, TEXT("DetectSnapTriggerBox"));
			bDetectSnabTrigger = DetectSnapTriggerBox(HitResult);
		}

		if (!bDetectSnabTrigger)
		{
			//UE_LOG(LogTrap, Warning, TEXT("bDetectSnabTrigger = false"));

			// �����ڽ��� ���̰� ���� �ʾҴٸ� ����Ʈ ����Ʈ�� ��ġ ����
			FVector TrapSpawnLocation = HitResult.ImpactPoint;
			PreviewMeshTransform.SetLocation(TrapSpawnLocation);
			FRotator TrapRotator = FRotator(0.f, PreviewMeshRotationYaw, 0.f) + CameraRotation.Rotator();
			PreviewMeshTransform.SetRotation(FQuat(0.f, 0.f, TrapRotator.Quaternion().Z, TrapRotator.Quaternion().W));

			bCanInstall = false;
		}
		else
		{
			//UE_LOG(LogTrap, Warning, TEXT("bDetectSnabTrigger = true"));
			//UE_LOG(LogTrap, Warning, TEXT("bCanRotation = false;"));

			bCanRotationGuillotine = false;

			if (CheckForOverlapExclusivehGuillotine())
			{
				//UE_LOG(LogTrap, Warning, TEXT("if (CheckForOverlap())"));
				bCanInstall = false;
			}

			if (TrapSnapTrigger)
			{
				/*AGuillotinePendulumTrap* Guiilotion = Cast<AGuillotinePendulumTrap>(TrapSnapTrigger->GetSnapGuillotine());
				if (Guiilotion)
				{
					Guiilotion->AllMaterialBlue();
				}*/

				if (Player->HasAuthority())
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("1575 MulticastRPCSnapGuillotineControl False"));

					if (bCanInstall)
					{
						MulticastRPCSnapGuillotineControl(TrapSnapTrigger->GetSnapGuillotine(), false, true);
					}
					else
					{
						MulticastRPCSnapGuillotineControl(TrapSnapTrigger->GetSnapGuillotine(), false, false);
					}

					MulticastRPCSetHiddenPreviewMesh(true);
				}
				else
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("1581 ServerRPCSnapGuillotineControl False"));

					if (bCanInstall)
					{
						ServerRPCSnapGuillotineControl(TrapSnapTrigger->GetSnapGuillotine(), false, true);
					}
					else
					{
						ServerRPCSnapGuillotineControl(TrapSnapTrigger->GetSnapGuillotine(), false, false);
					}

					ServerRPCSetHiddenPreviewMesh(true);
				}
			}
		}
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("HitResultActor->IsA(ATrapSnapTrigger::StaticClass()) False False"));
	}
}

bool UTrapMechanics::CheckForOverlap()
{
	// ���� ĳ����
	//FHitResult HitResult;
	TArray<FHitResult> HitResults;
	bool bHasHit = false;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(PreviewMesh->GetOwner());

	FVector ExtentResult;

	if (!PreviewMesh->GetStaticMesh())
	{
		return false;
	}

	// �� Ʈ���̽� (Bear)
	if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Bear)
	{
		ExtentResult = PreviewMesh->GetStaticMesh()->GetBounds().BoxExtent / 1.5f;
	}

	// �ڽ� Ʈ���̽�
	else
	{
		ExtentResult = PreviewMesh->GetStaticMesh()->GetBounds().BoxExtent / 1.1f;
	}

	FVector BoxLocation = PreviewMeshTransform.GetLocation();

	if (TrapPartsArray[TrapPartsTableIndex].bIsCeling)
	{
		FVector CellingBoxLocation = BoxLocation;
		if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::OilBag)
		{
			CellingBoxLocation.Z += 150;
		}
		else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Plank)
		{
			CellingBoxLocation.Z += 80;
		}

		TArray<FHitResult> CellingHitResults;

		bHasHit = UKismetSystemLibrary::BoxTraceMulti(
			this,
			CellingBoxLocation,
			CellingBoxLocation,
			ExtentResult,
			PreviewMeshTransform.GetRotation().Rotator(),
			UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel12),
			false,
			ActorsToIgnore,
			//EDrawDebugTrace::ForDuration,
			EDrawDebugTrace::None,
			CellingHitResults,
			true);

		if (bHasHit)
		{
			for (const auto& Result : CellingHitResults)
			{
				AActor* HitActor = Result.GetActor();
				if (IsValid(HitActor) && HitActor->ActorHasTag("Trap"))
				{
					return bHasHit;
				}
			}

			bHasHit = false;
		}
	}

	if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GunpowderBarrel ||
		TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
	{
		//UE_LOG(LogTrap, Warning, TEXT("ExtentResult.Z %f"), ExtentResult.Z);
		BoxLocation.Z += ExtentResult.Z;
	}

	if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Bear)
	{
		ExtentResult.Z = 150;
		BoxLocation.Z += 75.f;
	}

	bHasHit = UKismetSystemLibrary::BoxTraceMulti(
		this,
		BoxLocation,
		BoxLocation,
		ExtentResult,
		PreviewMeshTransform.GetRotation().Rotator(),
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel12),
		false,
		ActorsToIgnore,
		//EDrawDebugTrace::ForDuration,
		EDrawDebugTrace::None,
		HitResults,
		true);

	//UE_LOG(LogTrap, Warning, TEXT("CheckForOverlap HasHit = %s"), HasHit ? TEXT("true") : TEXT("false"));

	if (bHasHit)
	{
		for (const auto& Result : HitResults)
		{
			AActor* HitActor = Result.GetActor();
			if (IsValid(HitActor))
			{
				if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spear)
				{
					bHasHit = false;
				}
				else
				{
					if (PreviewMeshRotator.Roll == 0 && PreviewMeshRotator.Pitch == 0)
					{
						if (PreviewMeshTransform.GetLocation().Z < Player->GetActorLocation().Z)
						{
							bHasHit = false;
						}
						else
						{
							bHasHit = true;
						}

					}
					else
					{
						bHasHit = false;
					}
				}

				// ������ �������Ǿ��ִٸ� �����̶� ��ġ �Ұ� ����
				if (!bHasHit)
				{
					//if ((IsValid(HitActor) && HitActor->IsA(ATrapBase::StaticClass())))
					if ((IsValid(HitActor) && HitActor->ActorHasTag("Trap")))
					{
						bHasHit = true;
					}
				}

				if (bHasHit)
				{
					if (HitActor->ActorHasTag("GuillotinePendulumTrap") && Result.GetComponent()->ComponentHasTag("TrapCollision"))
					{
						if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spike ||
							TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Bear ||
							TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GunpowderBarrel ||
							TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Spear)
						{
							bHasHit = false;
						}
					}
					else if (HitActor->ActorHasTag("CheckClapPath"))
					{
						if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GunpowderBarrel)
						{
							bHasHit = false;
						}
					}
				}

				if (bHasHit)
				{
					break;
				}
			}
		}
	}

	if (!bHasHit)
	{
		if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GunpowderBarrel ||
			TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
		{
			TArray<FHitResult> HitResultsForClapAndBarrel;

			bHasHit = UKismetSystemLibrary::SphereTraceMulti(
				this,
				BoxLocation,
				BoxLocation,
				//PreviewMesh->GetStaticMesh()->GetBounds().BoxExtent.X + 200,
				300,
				UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel12),
				false,
				ActorsToIgnore,
				//EDrawDebugTrace::ForDuration,
				EDrawDebugTrace::None,
				HitResultsForClapAndBarrel,
				true);

			if (bHasHit)
			{
				bHasHit = false;

				for (const auto& Result : HitResultsForClapAndBarrel)
				{
					AActor* HitActor = Result.GetActor();
					if (IsValid(HitActor))
					{
						if (HitActor->ActorHasTag("GunpowderBarrelTrap") || HitActor->ActorHasTag("ClapTrap"))
						{
							//UE_LOG(LogTrap, Warning, TEXT("HitActor Name : %s"), *HitActor->GetName());
							bHasHit = true;
							break;
						}
					}
				}
			}
		}
	}

	return bHasHit;
}

bool UTrapMechanics::CheckForOverlapSpearWall()
{
	TArray<FHitResult> HitResults;
	FVector ExtentResult;
	bool bHasHit = false;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(PreviewMesh->GetOwner());

	if (!PreviewCouple->GetStaticMesh())
	{
		return false;
	}

	ExtentResult = PreviewCouple->GetStaticMesh()->GetBounds().BoxExtent / 1.1f;

	FVector BoxLocation = PreviewCoupleTransform.GetLocation();

	bHasHit = UKismetSystemLibrary::BoxTraceMulti(
		this,
		BoxLocation,
		BoxLocation,
		ExtentResult,
		PreviewMeshTransform.GetRotation().Rotator(),
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel12),
		false,
		ActorsToIgnore,
		//EDrawDebugTrace::ForDuration,
		EDrawDebugTrace::None,
		HitResults,
		true);

	if (bHasHit)
	{
		for (const auto& Result : HitResults)
		{
			AActor* HitActor = Result.GetActor();
			if (IsValid(HitActor))
			{
				if (HitActor->ActorHasTag("Trap"))
				{
					bHasHit = true;
				}
				else
				{
					bHasHit = false;
				}
			}

			if (bHasHit)
			{
				//UE_LOG(LogTrap, Warning, TEXT("bHasHit = true"));
				break;
			}
		}
	}

	return bHasHit;
}

void UTrapMechanics::AlignToSurface()
{
	// ��簢�� üũ�� ���� ����ĳ��Ʈ
	FHitResult GroundHit;
	FVector Start = PreviewMeshTransform.GetLocation();
	FVector End = Start - FVector(0, 0, 150);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, true);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(PreviewMesh->GetOwner());

	bool bGroundHit = UKismetSystemLibrary::LineTraceSingle(this, Start, End, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None, GroundHit, true);
	if (bGroundHit)
	{
		FVector Normal = GroundHit.ImpactNormal;
		float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));

		// �ִ� ��簢�� ���� (��: 45��)
		const float MaxSlopeAngle = 45.0f;
		const float MinSlopeAngle = 10.f;

		if (SlopeAngle > MaxSlopeAngle || SlopeAngle < MinSlopeAngle)
		{
			bAlignToSurface = false;
			return;
		}

		// ȭ������ ����� ���� ��ġ ����
		if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GunpowderBarrel)
		{
			if (SlopeAngle > 15.f)
			{
				return;
			}
		}

		if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
		{
			return;
			/*if (SlopeAngle > 30.f)
			{
				return;
			}*/
		}

		FVector ForwardVector = FVector::CrossProduct(FVector::RightVector, Normal).GetSafeNormal();
		FRotator Rotator = FRotationMatrix::MakeFromXZ(ForwardVector, Normal).Rotator();
		//UE_LOG(LogTrap, Warning, TEXT("Rotator : %s"), *Rotator.ToString());

		  // 로컬 Yaw 값을 변경하고자 하는 값
		float LocalYawOffset = PreviewMeshRotationYaw; // 예를 들어 변경하고자 하는 Yaw 값

		// BaseRotator를 쿼터니언으로 변환
		FQuat BaseQuat = FQuat(Rotator);

		// 로컬 Yaw 회전을 추가 (기준이 되는 노멀 벡터를 사용)
		FQuat LocalYawQuat = FQuat(Normal, FMath::DegreesToRadians(LocalYawOffset));

		// 최종 회전을 계산
		FQuat NewQuatRotation = LocalYawQuat * BaseQuat;

		PreviewMeshRotator = NewQuatRotation.Rotator();

		//PreviewMeshRotator = Rotator;
		bAlignToSurface = true;

		//UE_LOG(LogTrap, Warning, TEXT("Slope : %f degrees"), SlopeAngle);
		//UE_LOG(LogTrap, Warning, TEXT("Rotator : %s"), *Rotator.ToString());
	}

	//UE_LOG(LogTrap, Warning, TEXT("AlignToSurface bGroundHit = false"));
	return;
}

void UTrapMechanics::NotifyPlayerInstallationMode() const
{
	if (bInstallationMode)
	{
		Player->EnterInstallationMode();
	}
	else
	{
		Player->ReleaseInstallationMode();
	}
}

void UTrapMechanics::AllPreviewSetTransform()
{
	if (IsValid(PreviewMesh))
	{
		if (TrapPartsArray[TrapPartsTableIndex].bIsCouple)
		{
			if (IsValid(PreviewCouple))
			{
				PreviewCouple->SetWorldTransform(PreviewCoupleTransform);
			}
		}

		if (TrapPartsArray[TrapPartsTableIndex].bIsPreviewRange)
		{
			if (IsValid(PreviewRange))
			{
				PreviewRange->SetWorldTransform(PreviewRangeTransform);
			}

			if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GunpowderBarrel)
			{
				if (IsValid(PreviewNiagara))
				{
					//UE_LOG(LogTrap, Warning, TEXT("PreviewNiagara->SetActorTransform(PreviewRangeTransform);"));

					FTransform NiagaraTransform = PreviewMeshTransform;
					NiagaraTransform.SetRotation(FQuat::Identity);
					PreviewNiagara->SetActorTransform(NiagaraTransform);
				}
			}
			else if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Clap)
			{
				if (IsValid(PreviewRangeDecal) && !bCanInstallCouple)
				{
					FVector RangeDecalLocation = PreviewMeshTransform.GetLocation();
					PreviewRangeDecal->SetActorLocation(RangeDecalLocation);
				}
			}
		}

		//UE_LOG(LogTrap, Warning, TEXT("PreviewMeshRotation : %s"), *PreviewMeshTransform.Rotator().ToString());
		PreviewMesh->SetWorldTransform(PreviewMeshTransform);

		if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::GuillotinePendulum)
		{
			if (bCanRotationGuillotine)
			{
				//UE_LOG(LogTrap, Warning, TEXT("bCanRotationGuillotine"));
				//UE_LOG(LogTrap, Warning, TEXT("PreviewMeshRotationYaw : %f"),PreviewMeshRotationYaw);
				FRotator TestRotator = FRotator(0, PreviewMeshRotationYaw, 0);
				PreviewMesh->AddLocalRotation(TestRotator);
			}
		}
	}
}

void UTrapMechanics::InitializeTrap(ATrapBase* Trap)
{
	if (!Trap && !Player)
	{
		return;
	}

	if (Player->Is1Player())
	{
		Trap->SetIsInstalled1P(true);
	}
	else
	{
		Trap->SetIsInstalled1P(false);
	}

	// Ʈ�� Ÿ�� ����
	Trap->SetTrapType(TrapPartsArray[TrapPartsTableIndex].Type);

	// Ʈ�� �ֹ߼� ���� ����
	Trap->SetVolatile(TrapPartsArray[TrapPartsTableIndex].bVolatile);

	// TrapInfo ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ

	//float InstallCost = TrapPartsTable[TrapPartsTableIndex].InstallCost;
	float InstallCost = TrapDataArray[TrapPartsTableIndex].InstallCost;
	InstallCost = InstallCost * CostRate;
	//Trap->SetInstallCost(InstallCost);

	Trap->SetDataTableInfo(
		InstallCost,
		TrapDataArray[TrapPartsTableIndex].Damage,
		TrapDataArray[TrapPartsTableIndex].IncreaseDamage,
		TrapDataArray[TrapPartsTableIndex].Speed,
		TrapDataArray[TrapPartsTableIndex].IncreaseSpeed,
		TrapDataArray[TrapPartsTableIndex].Strength,
		TrapDataArray[TrapPartsTableIndex].IncreaseStrength,
		TrapDataArray[TrapPartsTableIndex].Duration);

	//UE_LOG(LogTrap, Warning, TEXT("TrapDataArray[TrapPartsTableIndex].Strength : %f"), TrapDataArray[TrapPartsTableIndex].Strength);
}

bool UTrapMechanics::CheckCellingTrap(FHitResult& HitResult, FVector& StartLocation, FVector& EndLocation, FVector& CameraForwardVector, FCollisionQueryParams& CollisionParams, bool& bHasHit)
{
	//UE_LOG(LogTrap, Warning, TEXT("HasHit CellingTrigger"));
				//UE_LOG(LogTrap, Warning, TEXT("HasHitName : %s"), *HitResult.GetActor()->GetName());

	AActor* HitActor = HitResult.GetActor();
	if (HitActor->ActorHasTag("CellingTrigger"))
	{
		//UE_LOG(LogTrap, Warning, TEXT("HitResult.GetActor()->ActorHasTag(CellingTrigger)"));

		StartLocation = EndLocation;
		float AddRayDistance = 0.f;

		if (HitActor->ActorHasTag("1.5"))
		{
			AddRayDistance = MaxInstallRayDistance * 0.5;
		}
		else if (HitActor->ActorHasTag("2"))
		{
			AddRayDistance = MaxInstallRayDistance;
		}
		else if (HitActor->ActorHasTag("2.5"))
		{
			AddRayDistance = MaxInstallRayDistance * 1.5;
		}
		else if (HitActor->ActorHasTag("3"))
		{
			AddRayDistance = MaxInstallRayDistance * 2;
		}
		else if (HitActor->ActorHasTag("3.5"))
		{
			AddRayDistance = MaxInstallRayDistance * 2.5;
		}
		else if (HitActor->ActorHasTag("4"))
		{
			AddRayDistance = MaxInstallRayDistance * 3;
		}
		else if (HitActor->ActorHasTag("5"))
		{
			AddRayDistance = MaxInstallRayDistance * 4;
		}

		// 2�� �ø���

		EndLocation += CameraForwardVector * AddRayDistance;

		//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 5.f);

		CollisionParams.AddIgnoredActor(HitActor);

		// ���� ĳ����
		return bHasHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, CollisionParams);
	}

	return false;
}

void UTrapMechanics::CheckSnapTrap(FHitResult& HitResult, bool& bDetectSnabTrigger, FQuat& CameraRotation)
{
	AActor* HitResultActor = HitResult.GetActor();

	// Ʈ�� ������ ������ ȯ�� ������Ʈ���� üũ 
	if (HitResultActor->IsA(ATrapSnabEnviromentBase::StaticClass()))
	{
		TrapSnapEnviromentActor = Cast<ATrapSnabEnviromentBase>(HitResultActor);

		// ���Ϳ� Ʈ���� �̹� ��ġ�Ǿ� �ִ��� üũ (�ߺ� ��ġ x)
		if (IsValid(TrapSnapEnviromentActor) && TrapSnapEnviromentActor->IsInstalledTrap())
		{
			//UE_LOG(LogTrap, Warning, TEXT("TrapSnapEnviromentActor->IsInstalledTrap()"));
			bDetectSnabTrigger = false;
		}
		else
		{
			bDetectSnabTrigger = DetectSnapTriggerBoxes(HitResult);
		}
	}

	if (!bDetectSnabTrigger)
	{
		// �����ڽ��� ���̰� ���� �ʾҴٸ� ����Ʈ ����Ʈ�� ��ġ ����
		FVector TrapSpawnLocation = HitResult.ImpactPoint;
		PreviewMeshTransform.SetLocation(TrapSpawnLocation);
		FRotator TrapRotator = FRotator(0.f, PreviewMeshRotationYaw, 0.f) + CameraRotation.Rotator();
		PreviewMeshTransform.SetRotation(FQuat(0.f, 0.f, TrapRotator.Quaternion().Z, TrapRotator.Quaternion().W));

		/// ���ƾ
		// ���� ȯ�� ������Ʈ�� �����ϴ� Ʈ���� �⺻������ ��ġ �Ұ���
		if (/*TrapType:: == TrapPartsTable[TrapPartsTableIndex].Type ||*/
			ETrapType::GuillotinePendulum == TrapPartsArray[TrapPartsTableIndex].Type)
		{
			bCanInstall = false;
		}
	}
	else
	{
		bool TriggerEmpty = TrapSnapEnviromentActor->IsCanInstall();
		if (!TriggerEmpty)
		{
			bCanInstall = false;
			return;
		}
	}
}

void UTrapMechanics::CheckServerBroadCastSetHiddenRange(bool IsServer)
{
	if (bCanInstall)
	{
		if (IsServer)
		{
			MulticastRPCSetHiddenNiagara(false);
		}
		else
		{
			ServerRPCSetHiddenNiagara(false);
		}
	}
	else
	{
		if (IsServer)
		{
			MulticastRPCSetHiddenNiagara(true);
		}
		else
		{
			ServerRPCSetHiddenNiagara(true);
		}
	}
}

void UTrapMechanics::CalculateRangeMeshTransform()
{
	// ��簢�� üũ�� ���� ����ĳ��Ʈ
	FHitResult GroundHit;
	FVector Start = PreviewMeshTransform.GetLocation();
	FVector End = Start + FVector(0, 0, -3000);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, true);

	FCollisionQueryParams PlankRayCollisionParams;
	PlankRayCollisionParams.AddIgnoredActor(PreviewMesh->GetOwner());
	PlankRayCollisionParams.AddIgnoredComponent(PreviewMesh);

	FTransform RangeTransform = PreviewMesh->GetComponentTransform();
	FVector RangeLocation = RangeTransform.GetLocation();

	bool bGroundHit = GetWorld()->LineTraceSingleByChannel(GroundHit, Start, End, ECC_GameTraceChannel8, PlankRayCollisionParams);
	if (bGroundHit)
	{
		RangeLocation.Z = GroundHit.ImpactPoint.Z;

		/*UE_LOG(LogTrap, Warning, TEXT("------------------------------"));
		UE_LOG(LogTrap, Warning, TEXT("PreviewMesh Location : %s"), *RangeLocation.ToString());
		UE_LOG(LogTrap, Warning, TEXT("GroundHit.ImpactPoint.Z : %f"), GroundHit.ImpactPoint.Z);
		UE_LOG(LogTrap, Warning, TEXT("PreviewMesh Transform : %s"), *PreviewMeshTransform.ToString());
		UE_LOG(LogTrap, Warning, TEXT("PreviewRange Transform : %s"), *RangeTransform.ToString());
		UE_LOG(LogTrap, Warning, TEXT("------------------------------"));*/
	}
	RangeLocation.Z += 10;
	RangeTransform.SetLocation(RangeLocation);
	if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Plank)
	{
		RangeTransform.SetScale3D(FVector(2, 2, 0));
	}
	if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::OilBag)
	{
		RangeTransform.SetScale3D(FVector(5, 5, 0));
	}
	PreviewRangeTransform = RangeTransform;
}

bool UTrapMechanics::CheckForOverlapExclusivehGuillotine()
{
	FHitResult HitResult;
	TArray<FHitResult> HitResults;

	bool bHasHit = false;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(TrapSnapTrigger->GetSnapGuillotine());

	FVector ExtentResult;
	FVector BoxLocation;

	AGuillotinePendulumTrap* Guiilotion = Cast<AGuillotinePendulumTrap>(TrapSnapTrigger->GetSnapGuillotine());
	if (Guiilotion)
	{
		for (int i = 0; i < 4; ++i)
		{
			FRotator BoxRotator = PreviewMeshTransform.Rotator();

			if (i == 0)
			{
				ExtentResult = Guiilotion->GetMesh()->GetStaticMesh()->GetBounds().BoxExtent;
				BoxLocation = Guiilotion->GetMesh()->GetComponentLocation();
				BoxLocation.Z -= 430;
				ExtentResult.Y += 500;
				//BoxRotator = Guiilotion->GetMesh()->GetComponentRotation();
			}
			else if (i == 1)
			{
				ExtentResult = Guiilotion->GetBeam()->GetStaticMesh()->GetBounds().BoxExtent;
				BoxLocation = Guiilotion->GetBeam()->GetComponentLocation();
				BoxLocation.Z += 85;
			}
			else if (i == 2)
			{
				ExtentResult = Guiilotion->GetPanel1()->GetStaticMesh()->GetBounds().BoxExtent;
				BoxLocation = Guiilotion->GetPanel1()->GetComponentLocation();
			}
			else if (i == 3)
			{
				ExtentResult = Guiilotion->GetPanel2()->GetStaticMesh()->GetBounds().BoxExtent;
				BoxLocation = Guiilotion->GetPanel2()->GetComponentLocation();
			}

			if (i == 0)
			{
				bHasHit = UKismetSystemLibrary::BoxTraceMulti(
					this,
					BoxLocation,
					BoxLocation,
					ExtentResult,
					BoxRotator,
					UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel12),
					false,
					ActorsToIgnore,
					//EDrawDebugTrace::ForDuration,
					EDrawDebugTrace::None,
					HitResults,
					true);
			}

			else
			{
				bHasHit = UKismetSystemLibrary::BoxTraceSingle(
					this,
					BoxLocation,
					BoxLocation,
					ExtentResult,
					BoxRotator,
					UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel12),
					false,
					ActorsToIgnore,
					//EDrawDebugTrace::ForDuration,
					EDrawDebugTrace::None,
					HitResult,
					true);
			}

			if (i == 0)
			{
				if (bHasHit)
				{
					for (const auto& Result : HitResults)
					{
						AActor* HitActor = Result.GetActor();
						if (HitActor->IsA(ATrapBase::StaticClass()))
						{
							if (HitActor->ActorHasTag("ClapTrap") ||
								HitActor->ActorHasTag("OilBagTrap") ||
								HitActor->ActorHasTag("PlankTrap"))
							{
								return bHasHit;
							}
							else
							{
								continue;
							}
						}
					}
				}
			}
			else
			{
				if (bHasHit)
				{
					AActor* HitActor = HitResult.GetActor();
					if (HitActor->IsA(ATrapBase::StaticClass()))
					{
						return bHasHit;
					}
					else
					{
						bHasHit = false;
					}
				}
			}
		}

		return bHasHit;
	}

	return false;
}

void UTrapMechanics::DemolishTrap()
{
	if (!Player->IsLocallyControlled() || !FocusTrap || bInstallationMode)
	{
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("DemolishTrap"));

	if (Player->HasAuthority())
	{
		DestroyFocusTrap();
	}
	else
	{
		ServerRPCDestroyFocusTrap(FocusTrap);
	}

	if (FocusTrap->ActorHasTag("GuillotinePendulumTrap"))
	{
		TArray<UStaticMeshComponent*> MeshComponents;
		FocusTrap->GetComponents<UStaticMeshComponent>(MeshComponents);

		for (UStaticMeshComponent* Mesh : MeshComponents)
		{
			Mesh->SetRenderCustomDepth(false);
		}
	}

	FocusTrap = nullptr;
	bHitRayCanReInstall = false;

	PlayerController->PlayerHudRef->SetTrapWeckageFrameVisibility(false);
	PlayerController->PlayerHudRef->SetTrapDemolishFrameVisibility(false);
}

void UTrapMechanics::DestroyFocusTrap()
{
	if (!IsValid(FocusTrap))
	{
		return;
	}

	if (!FocusTrap->IsWeckage())
	{
		int32 ReturnCost = FocusTrap->GetInstallCost() * 0.8;

		//UE_LOG(LogTrap, Warning, TEXT("TrapCost : %d"), FocusTrap->GetInstallCost());
		//UE_LOG(LogTrap, Warning, TEXT("ReturnCost : %d"), ReturnCost);

		if (Player->HasAuthority())
		{
			Player->AddBoneItem(ReturnCost);
		}
		else
		{
			Player->ServerRPCAddBoneItem(ReturnCost);
		}
	}

	FocusTrap->Demolish();

	if (FocusTrap->ActorHasTag("GuillotinePendulumTrap"))
	{
		//UE_LOG(LogTrap, Warning, TEXT("GuillotinePendulumTrap"));
		FocusTrap->SetActorEnableCollision(false);
		FocusTrap->SetActorHiddenInGame(true);
		AActor* TriggerActor = FocusTrap->GetOwner();
		TrapSnapTrigger = Cast<ATrapSnapTrigger>(TriggerActor);
		if (IsValid(TrapSnapTrigger))
		{
			MulticastRPCSnapTriggerSetTrapNullptr(TrapSnapTrigger, FocusTrap);
		}
	}
	else if (FocusTrap->ActorHasTag("ClapTrap"))
	{
		AClapTrap* ClapTrap = Cast<AClapTrap>(FocusTrap);
		if (IsValid(ClapTrap))
		{
			AClapTrap* OtherClapTrap = ClapTrap->GetOtherClap();
			if (IsValid(OtherClapTrap))
			{
				if (ClapTrap->Children.Num() > 0 && IsValid(ClapTrap->Children[0]))
				{
					//UE_LOG(LogTrap, Warning, TEXT("First Clap Child True And Destroy"));

					ClapTrap->Children[0]->Destroy();
				}
				else if (OtherClapTrap->Children.Num() > 0 && IsValid(OtherClapTrap->Children[0]))
				{
					//UE_LOG(LogTrap, Warning, TEXT("Second Clap Child True And Destroy"));

					OtherClapTrap->Children[0]->Destroy();
				}

				OtherClapTrap->Destroy();

				if (IsValid(TrapperGameState.Get()))
				{
					//UE_LOG(LogTrap, Warning, TEXT("GameState ClapTrap true"));
					TrapperGameState->NextCanPlaceClapTrap(true);
				}
			}

			ClapTrap->Destroy();
		}
	}
	else
	{
		if (FocusTrap->ActorHasTag("SpikeTrap"))
		{
			DecreaseTrapHudNum(ETrapType::Spike, false);
		}
		else if (FocusTrap->ActorHasTag("GunpowderBarrelTrap"))
		{
			DecreaseTrapHudNum(ETrapType::GunpowderBarrel, false);
		}

		FocusTrap->Destroy();
	}
}

void UTrapMechanics::ReInstallTrap()
{
	if (!Player->IsLocallyControlled() || !FocusTrap || !FocusTrap->IsWeckage() || bInstallationMode || bSelectMode)
	{
		return;
	}

	//UE_LOG(LogTrap, Warning, TEXT("ReInstallTrap"));
	//UE_LOG(LogTrap, Warning, TEXT("FocusTrap Cost : %d"), FocusTrap->GetInstallCost());
	//UE_LOG(LogTrap, Warning, TEXT("Player BoneItemBox : %d"), Player->BoneItemBox);

	bool bCanReInstall = true;
	if (Player->BoneItemBox < FocusTrap->GetInstallCost())
	{
		//UE_LOG(LogTrap, Warning, TEXT("BoneItemBox false"));
		UGameplayStatics::PlaySoundAtLocation(this, InstallFailSound, Player->GetActorLocation());
		return;
	}

	/*if (FocusTrap->ActorHasTag("SpikeTrap"))
	{
		if (Player->IsInstallSpikeTrap())
		{
			Player->ClientRPCUpdateTrapHudNum(ETrapType::Spike, true);
		}
		else
		{
			bCanReInstall = false;
		}
	}
	else if (FocusTrap->ActorHasTag("GunpowderBarrelTrap"))
	{
		if (Player->IsInstallGunpowderBarrelTrap())
		{
			Player->ClientRPCUpdateTrapHudNum(ETrapType::GunpowderBarrel, true);
		}
		else
		{
			bCanReInstall = false;
		}
	}
	else if (FocusTrap->ActorHasTag("ClapTrap"))
	{
		if (IsValid(TrapperGameState.Get()))
		{
			if (TrapperGameState->IsCanPlaceClapTrap())
			{
				TrapperGameState->NextCanPlcaeClapTrap(false);
			}
			else
			{
				bCanReInstall = false;
			}
		}
	}*/

	//if (!bCanReInstall)
	//{
	//	if (InstallFailSound)
	//	{
	//		UE_LOG(LogTrap, Warning, TEXT("aa"));
	//		UGameplayStatics::PlaySoundAtLocation(this, InstallFailSound, Player->GetActorLocation());
	//	}
	//	return;
	//}

	Player->BlockBow();
	Player->UseBoneItem(FocusTrap->GetInstallCost());

	//UE_LOG(LogTemp, Warning, TEXT("UseBoneItem true"));

	if (Player->HasAuthority())
	{
		PlaySoundInstallTrap();

		MulticastRPCReInstallTrap();

		// Trap Anim
		MulticastRPCStartInstallAnim();
		FTimerHandle AnimHandle;
		GetWorld()->GetTimerManager().SetTimer(AnimHandle, this, &UTrapMechanics::AnimMulticastTimerCallback, 1.14, false);
	}
	else
	{
		PlaySoundInstallTrap();

		ServerRPCReInstallTrap(FocusTrap);

		// Trap Anim
		Player->StartInstallAnim();
		ServerRPCStartInstallAnim();

		FTimerHandle AnimHandle;
		GetWorld()->GetTimerManager().SetTimer(AnimHandle, this, &UTrapMechanics::AnimServerTimerCallback, 1.14, false);
	}

	//FocusTrap = nullptr;
	bHitRayCanReInstall = false;

	/*PlayerController->PlayerHudRef->SetTrapWeckageFrameVisibility(false);
	PlayerController->PlayerHudRef->SetTrapDemolishFrameVisibility(true);*/
}

void UTrapMechanics::PlaySoundInstallTrap()
{
	if (Player->InteractionSound)
	{
		// 특정 지점에서 사운드 재생
		UGameplayStatics::PlaySoundAtLocation(this, Player->InteractionSound, Player->GetActorLocation());
	}
}

void UTrapMechanics::SafeNormalizeQuat()
{
	PreviewMeshTransform.NormalizeRotation();
	PreviewCoupleTransform.NormalizeRotation();
	PreviewRangeTransform.NormalizeRotation();

	//PreviewMeshTransform.GetRotation().Normalize();
}

void UTrapMechanics::ClientDestroyPreviweMeshTimerCallback()
{
	//UE_LOG(LogTrap, Warning, TEXT("Destroy Timer"));
	if (IsValid(PreviewMesh))
	{
		//UE_LOG(LogTrap, Warning, TEXT("Clear Timer"));
		GetWorld()->GetTimerManager().ClearTimer(ClientDestroyPreviweMeshTimerHandle);
		SafeReleasePreviewMesh();
		ServerRPCRealesePreviewMesh(TrapSnapTrigger);
	}
	else
	{
		//UE_LOG(LogTrap, Warning, TEXT("Clear Timer Fail"));
	}
}

void UTrapMechanics::AnimMulticastTimerCallback()
{
	MulticastRPCEndInstallAnim();

}

void UTrapMechanics::AnimServerTimerCallback()
{
	Player->EndInstallAnim();
	ServerRPCEndInstallAnim();
}

void UTrapMechanics::BlockSelectTrap()
{
	bBlockSelectTrap = true;
}

void UTrapMechanics::UnBlockSelectTrap()
{
	bBlockSelectTrap = false;
}

void UTrapMechanics::DemolishReInstallRaycastLoop()
{
	//UE_LOG(LogTrap, Warning, TEXT("DemolishRaycastLoop"));

	if (!IsValid(Player) || !Player->IsLocallyControlled() || !IsValid(PlayerController) || !IsValid(PlayerController->PlayerHudRef))
	{
		return;
	}

	if (bInstallationMode || bSelectMode)
	{
		return;
	}

	FTransform CameraWorldTransform = FollowCamera->GetComponentTransform();
	FVector CameraLocation = CameraWorldTransform.GetLocation();
	FQuat CameraRotation = CameraWorldTransform.GetRotation();
	FVector CameraForwardVector = CameraRotation.GetForwardVector();
	FVector StartLocation = CameraLocation + CameraForwardVector * FVector::Distance(Player->GetActorLocation(), CameraLocation);
	FVector EndLocation = StartLocation + CameraForwardVector * MaxDemolishReInstallRayDistance;

	//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, true);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());

	FHitResult HitResult;
	bool bHasHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel11, CollisionParams);
	bool bCellingTrap = false;

	if (bHasHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor->ActorHasTag("CellingTrigger"))
		{
			bCellingTrap = true;

			//UE_LOG(LogTrap, Warning, TEXT("CellingTrigger"));

			StartLocation = EndLocation;
			float AddRayDistance = 0.f;

			if (HitActor->ActorHasTag("1.5"))
			{
				AddRayDistance = MaxInstallRayDistance * 0.5;
			}
			else if (HitActor->ActorHasTag("2"))
			{
				AddRayDistance = MaxInstallRayDistance;
			}
			else if (HitActor->ActorHasTag("2.5"))
			{
				AddRayDistance = MaxInstallRayDistance * 1.5;
			}
			else if (HitActor->ActorHasTag("3"))
			{
				AddRayDistance = MaxInstallRayDistance * 2;
			}
			else if (HitActor->ActorHasTag("3.5"))
			{
				AddRayDistance = MaxInstallRayDistance * 2.5;
			}
			else if (HitActor->ActorHasTag("4"))
			{
				AddRayDistance = MaxInstallRayDistance * 3;
			}
			else if (HitActor->ActorHasTag("5"))
			{
				AddRayDistance = MaxInstallRayDistance * 4;
			}

			EndLocation += CameraForwardVector * AddRayDistance;
			CollisionParams.AddIgnoredActor(HitActor);

			//DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, true);

			bHasHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel11, CollisionParams);
		}
	}

	//FocusTrap = nullptr;
	bHitRayCanReInstall = false;

	PlayerController->PlayerHudRef->SetTrapWeckageFrameVisibility(false);
	PlayerController->PlayerHudRef->SetTrapDemolishFrameVisibility(false);

	if (IsValid(FocusTrap) && FocusTrap->IsOutline())
	{
		TArray<UStaticMeshComponent*> MeshComponents;
		FocusTrap->GetComponents<UStaticMeshComponent>(MeshComponents);

		for (UStaticMeshComponent* Mesh : MeshComponents)
		{
			Mesh->SetRenderCustomDepth(false);
		}

		FocusTrap->SetOutline(false);

		if (FocusTrap->ActorHasTag("ClapTrap"))
		{
			AClapTrap* ClapTrap = Cast<AClapTrap>(FocusTrap);
			if (ClapTrap)
			{
				ClapTrap = ClapTrap->GetOtherClap();
				if (ClapTrap)
				{
					TArray<UStaticMeshComponent*> OtherClapMeshComponents;
					ClapTrap->GetComponents<UStaticMeshComponent>(OtherClapMeshComponents);

					for (UStaticMeshComponent* OtherClapMesh : OtherClapMeshComponents)
					{
						OtherClapMesh->SetRenderCustomDepth(false);
					}
				}
			}
		}
	}

	if (bHasHit)
	{
		//UE_LOG(LogTrap, Warning, TEXT("bHasHit true"));

		AActor* HitActor = HitResult.GetActor();

		if (!HitActor->ActorHasTag("Trap"))
		{
			PlayerController->PlayerHudRef->SetTrapWeckageFrameVisibility(false);
			PlayerController->PlayerHudRef->SetTrapDemolishFrameVisibility(false);
			FocusTrap = nullptr;

			return;
		}

		if (bCellingTrap)
		{
			if (!HitActor->ActorHasTag("PlankTrap") && !HitActor->ActorHasTag("OilBagTrap"))
			{
				FocusTrap = nullptr;
				return;
			}
		}

		ATrapBase* HitTrap = Cast<ATrapBase>(HitActor);
		FocusTrap = HitTrap;

		if (FocusTrap->bTutorialTrap)
		{
			FocusTrap = nullptr;
			return;
		}

		// UI On
		if (FocusTrap->IsWeckage())
		{
			bHitRayCanReInstall = true;

			//UE_LOG(LogTrap, Warning, TEXT("Weckage true"));
			PlayerController->PlayerHudRef->SetTrapWeckageFrameVisibility(true);
		}
		else
		{
			//UE_LOG(LogTrap, Warning, TEXT("demolish true"));
			PlayerController->PlayerHudRef->SetTrapDemolishFrameVisibility(true);

			if (!FocusTrap->IsOutline())
			{
				TArray<UStaticMeshComponent*> MeshComponents;
				FocusTrap->GetComponents<UStaticMeshComponent>(MeshComponents);

				for (UStaticMeshComponent* Mesh : MeshComponents)
				{
					Mesh->SetRenderCustomDepth(true);
					Mesh->SetCustomDepthStencilValue(2);
				}

				FocusTrap->SetOutline(true);

				if (FocusTrap->ActorHasTag("ClapTrap"))
				{
					AClapTrap* ClapTrap = Cast<AClapTrap>(FocusTrap);
					if (ClapTrap)
					{
						ClapTrap = ClapTrap->GetOtherClap();
						if (ClapTrap)
						{
							TArray<UStaticMeshComponent*> OtherClapMeshComponents;
							ClapTrap->GetComponents<UStaticMeshComponent>(OtherClapMeshComponents);

							for (UStaticMeshComponent* OtherClapMesh : OtherClapMeshComponents)
							{
								OtherClapMesh->SetRenderCustomDepth(true);
								OtherClapMesh->SetCustomDepthStencilValue(2);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		//UE_LOG(LogTrap, Warning, TEXT("bHasHit false"));

		FocusTrap = nullptr;
	}
}

void UTrapMechanics::UpgradeCost(float CostIcreaseDecreaseRate)
{
	CostRate = CostIcreaseDecreaseRate;
}

void UTrapMechanics::CheckOnHovered()
{
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	FVector2D Center = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

	FVector2D MousePosition;
	GEngine->GameViewport->GetMousePosition(MousePosition);

	float Distance = FVector2D::Distance(Center, MousePosition);
	float Size = PlayerController->TrapSelectHUDRef->GetSize().X;
	float Radius = Size * 1 /*1.2*/;

	ETrapType CurrentHoveredTrapType = ETrapType::None;

	if (Distance < Radius)
	{
		CurrentHoveredTrapType = ETrapType::None;
		bCanSelect = false;
	}
	else
	{
		FVector2D Normal = MousePosition - Center;

		float Radians = FMath::Atan2(Normal.Y, Normal.X);
		float Degrees = FMath::RadiansToDegrees(Radians); //-180 ~ 180

		if (-180 < Degrees && Degrees <= -135)
		{
			CurrentHoveredTrapType = ETrapType::Spear;
		}
		else if (-135 < Degrees && Degrees <= -90)
		{
			CurrentHoveredTrapType = ETrapType::GuillotinePendulum;
		}
		else if (-90 < Degrees && Degrees <= -45)
		{
			CurrentHoveredTrapType = ETrapType::GunpowderBarrel;
		}
		else if (-45 < Degrees && Degrees <= 0)
		{
			CurrentHoveredTrapType = ETrapType::Plank;
		}
		else if (0 < Degrees && Degrees <= 45)
		{
			CurrentHoveredTrapType = ETrapType::Clap;
		}
		else if (45 < Degrees && Degrees <= 90)
		{
			CurrentHoveredTrapType = ETrapType::OilBag;
		}
		else if (90 < Degrees && Degrees <= 135)
		{
			CurrentHoveredTrapType = ETrapType::Bear;
		}
		else if (135 < Degrees && Degrees <= 180)
		{
			CurrentHoveredTrapType = ETrapType::Spike;
		}

		TrapPartsTableIndex = static_cast<uint8>(CurrentHoveredTrapType);
		//ItemCost = TrapPartsTable[TrapPartsTableIndex].InstallCost * CostRate;
		ItemCost = TrapDataArray[TrapPartsTableIndex].InstallCost * CostRate;
		bCanSelect = true;

		/*UE_LOG(LogTrap, Warning, TEXT("InstallCost : %d"), TrapPartsTable[TrapPartsTableIndex].InstallCost);
		UE_LOG(LogTrap, Warning, TEXT("CostRate : %f"), CostRate);
		UE_LOG(LogTrap, Warning, TEXT("Upgrade InstallCost : %d"), ItemCost);
		UE_LOG(LogTrap, Warning, TEXT("ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ"));*/
	}

	if (CurrentHoveredTrapType != HoveredTrapType)
	{
		if (Player->BoneItemBox >= ItemCost)
		{
			if (CurrentHoveredTrapType == ETrapType::Spike)
			{
				if (!Player->IsInstallSpikeTrap())
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Disabled);
				}
				else
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Enabled);
				}
			}
			else if (CurrentHoveredTrapType == ETrapType::GunpowderBarrel)
			{
				if (!Player->IsInstallGunpowderBarrelTrap())
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Disabled);
				}
				else
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Enabled);
				}
			}
			else if (CurrentHoveredTrapType == ETrapType::Clap)
			{
				if (!TrapperGameState->IsCanPlaceClapTrap())
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Disabled);
				}
				else
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Enabled);
				}
			}
			else
			{
				PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Enabled);
			}
		}
		else
		{
			PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Disabled);
		}


		if (CurrentHoveredTrapType != ETrapType::None
			&& SelectOverSound && Player->AudioComponent)
		{
			Player->AudioComponent->SetSound(SelectOverSound);
			Player->AudioComponent->Play();
		}
	}

	else if (CurrentHoveredTrapType == HoveredTrapType && LastItemBoxCount != Player->BoneItemBox)
	{
		if (Player->BoneItemBox >= ItemCost)
		{
			if (CurrentHoveredTrapType == ETrapType::Spike)
			{
				if (!Player->IsInstallSpikeTrap())
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Disabled);
				}
				else
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Enabled);
				}
			}
			else if (CurrentHoveredTrapType == ETrapType::GunpowderBarrel)
			{
				if (!Player->IsInstallGunpowderBarrelTrap())
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Disabled);
				}
				else
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Enabled);
				}
			}
			else if (CurrentHoveredTrapType == ETrapType::Clap)
			{
				if (!TrapperGameState->IsCanPlaceClapTrap())
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Disabled);
				}
				else
				{
					PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Enabled);
				}
			}
			else
			{
				PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Enabled);
			}
		}
		else
		{
			PlayerController->TrapSelectHUDRef->OnHovered(CurrentHoveredTrapType, ETrapHUDType::E_Disabled);
		}
	}

	PlayerController->TrapSelectHUDRef->LerpAngle(0.03);
	HoveredTrapType = CurrentHoveredTrapType;
	LastItemBoxCount = Player->BoneItemBox;
}

void UTrapMechanics::OnRep_PreviewMesh()
{
	if (IsValid(PreviewMesh))
	{
		//UE_LOG(LogTrap, Warning, TEXT("OnRep_PreviewMesh True"));
		// ������ �޽� �ʱ�ȭ
		//PreviewMesh->RegisterComponent();
		PreviewMesh->SetStaticMesh(TrapPartsArray[TrapPartsTableIndex].PreviewMesh);
		PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//PreviewMesh->SetHiddenInGame(true);
	}
	else
	{
		//UE_LOG(LogTrap, Warning, TEXT("OnRep_PreviewMesh false"));
	}
}

void UTrapMechanics::OnRep_PreviewRange()
{
	if (IsValid(PreviewRange))
	{
		//PreviewRange->RegisterComponent();
		PreviewRange->SetStaticMesh(TrapPartsArray[TrapPartsTableIndex].PreviewRange);
		PreviewRange->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		/*if (TrapPartsTable[TrapPartsTableIndex].Type == TrapType::GunpowderBarrel)
		{
			PreviewRange->SetHiddenInGame(true);
			PreviewRange->SetWorldScale3D(FVector(8.f, 8.f, 0.f));
		}*/

		if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::Plank)
		{
			PreviewRange->SetWorldScale3D(FVector(2.f, 2.f, 0.f));
		}

		if (TrapPartsArray[TrapPartsTableIndex].Type == ETrapType::OilBag)
		{
			PreviewRange->SetWorldScale3D(FVector(5.f, 5.f, 0.f));
		}

		PreviewRange->SetMaterial(0, PreviewMaterialBlue);
	}
}

void UTrapMechanics::OnRep_PreviewCouple()
{
	if (IsValid(PreviewCouple))
	{
		PreviewMeshRotationYaw = 90;

		//UE_LOG(LogTrap, Warning, TEXT("OnRep_PreviewCouple"));
		//PreviewCouple->RegisterComponent();
		PreviewCouple->SetStaticMesh(TrapPartsArray[TrapPartsTableIndex].PreviewCouple);
		PreviewCouple->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void UTrapMechanics::OnRep_PreviewNiagara()
{
	//UE_LOG(LogTrap, Warning, TEXT("OnRep_PreviewNiagara"));
	if (IsValid(PreviewNiagara))
	{
		PreviewNiagara->SetActorEnableCollision(ECollisionEnabled::NoCollision);
	}
}

void UTrapMechanics::OnRep_PreviewRangeDecal()
{
	//UE_LOG(LogTrap, Warning, TEXT("OnRep_PreviewRangeDecal"));

	/*if (PreviewRangeDecal)
	{
		UE_LOG(LogTrap, Warning, TEXT("OnRep_PreviewRangeDecal True"));

		PreviewRangeDecal->SetActorEnableCollision(ECollisionEnabled::NoCollision);
		PreviewRangeDecal->SetActorHiddenInGame(true);
	}*/
}

void UTrapMechanics::OnRep_TrapPartsTableIndex()
{
	bCanSnap = TrapPartsArray[TrapPartsTableIndex].bCanSnap;
	CollisionChannel = ConvertTraceTypeToCollisionChannel(TrapPartsArray[TrapPartsTableIndex].TraceChannel);
}

void UTrapMechanics::OnRep_PreviewMeshRotationYaw()
{
	/*UE_LOG(LogTrap, Warning, TEXT("OnRep_PreviewMeshRotationYaw"));
	UE_LOG(LogTrap, Warning, TEXT("PreviewMeshRotationYaw : %f"), PreviewMeshRotationYaw);*/
}

void UTrapMechanics::ServerRPCSetPreviewMesh_Implementation(uint8 TableIndex)
{
	//UE_LOG(LogTrap, Warning, TEXT("ServerRPCSetPreviewMesh_Implementation"));
	TrapPartsTableIndex = TableIndex;
	SetPreviewMesh();
}

void UTrapMechanics::ServerRPCUpdatePreviewMeshTransform_Implementation(FTransform PMTransform, FTransform PCTransform, FTransform PRTransform, float PMRotationYaw)
{
	//PMTransform.NormalizeRotation();
	//PCTransform.NormalizeRotation();
	//PRTransform.NormalizeRotation();

	//UE_LOG(LogTrap, Warning, TEXT("PCTransform : %s"), *PCTransform.ToString());

	PreviewMeshTransform = PMTransform;
	PreviewCoupleTransform = PCTransform;
	PreviewRangeTransform = PRTransform;
	PreviewMeshRotationYaw = PMRotationYaw;

	AllPreviewSetTransform();
}

void UTrapMechanics::ServerRPCUpdatePreviewMeshMaterial_Implementation(bool bMaterialBlue, class ATrapSnapTrigger* TSTrigger)
{
	MulticastRPCUpdatePreviewMeshMaterial(bMaterialBlue, TSTrigger);
}

void UTrapMechanics::MulticastRPCUpdatePreviewMeshMaterial_Implementation(bool bMaterialBlue, class ATrapSnapTrigger* TSTrigger)
{
	SetPreviewMeshMaterial(bMaterialBlue, TSTrigger);
}

void UTrapMechanics::ServerRPCRealesePreviewMesh_Implementation(class ATrapSnapTrigger* TSTrigger)
{
	//UE_LOG(LogTrap, Warning, TEXT("ServerRPCRealesePreviewMesh_Implementation"));
	TrapSnapTrigger = TSTrigger;

	SafeReleasePreviewMesh();
	ReleasePreviewMesh();
}

void UTrapMechanics::ServerRPCPlaceTrap_Implementation(class ATrapSnabEnviromentBase* ClientTrapSnabEnviromentActor, class ATrapSnapTrigger* ClientTrapSnabTrigger)
{
	TrapSnapEnviromentActor = ClientTrapSnabEnviromentActor;
	TrapSnapTrigger = ClientTrapSnabTrigger;
	bool bInstalledTrap = PermissionPlaceTrap();
	if (bInstalledTrap)
	{
		ClientRPCPlaySoundInstallTrap();
	}
}

void UTrapMechanics::ServerRPCSetHiddenNiagara_Implementation(bool Value)
{
	MulticastRPCSetHiddenNiagara(Value);
}

void UTrapMechanics::MulticastRPCSetHiddenNiagara_Implementation(bool Value)
{
	if (IsValid(PreviewNiagara))
	{
		//PreviewRange->SetHiddenInGame(Value);
		//PreviewNiagara->SetHidden(Value);
		PreviewNiagara->SetActorHiddenInGame(Value);
	}
}

void UTrapMechanics::ServerRPCSnapGuillotineControl_Implementation(ATrapBase* SnapGuillotine, bool bIsHidden, bool bMaterialBlue)
{
	MulticastRPCSnapGuillotineControl(SnapGuillotine, bIsHidden, bMaterialBlue);
}

void UTrapMechanics::MulticastRPCSnapGuillotineControl_Implementation(ATrapBase* SnapGuillotine, bool bIsHidden, bool bMaterialBlue)
{
	if (SnapGuillotine)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SnapGuillotine True"));
		SnapGuillotine->SetActorHiddenInGame(bIsHidden);
	}

	AGuillotinePendulumTrap* Guiilotion = Cast<AGuillotinePendulumTrap>(SnapGuillotine);
	if (bMaterialBlue)
	{
		if (Guiilotion)
		{
			Guiilotion->AllMaterialBlue();
		}
	}
	else
	{
		if (Guiilotion)
		{
			Guiilotion->AllMaterialRed();
		}
	}
}

void UTrapMechanics::ServerRPCSetHiddenPreviewMesh_Implementation(bool bIsHidden)
{
	MulticastRPCSetHiddenPreviewMesh(bIsHidden);
}

void UTrapMechanics::MulticastRPCSetHiddenPreviewMesh_Implementation(bool bIsHidden)
{
	if (IsValid(PreviewMesh))
	{
		PreviewMesh->SetHiddenInGame(bIsHidden);
	}
}

void UTrapMechanics::ServerRPCDestroyFocusTrap_Implementation(ATrapBase* Trap)
{
	FocusTrap = Trap;
	DestroyFocusTrap();
}

void UTrapMechanics::MulticastRPCReInstallTrap_Implementation()
{
	if (IsValid(FocusTrap))
	{
		FocusTrap->ReInstall();
		//MulticastRPCPlaySoundInstallTrap();

		if (IsValid(Player) && Player->HasAuthority() && TrapperGameState.Get())
		{
			if (Player->Is1Player())
			{
				TrapperGameState->AddScore(EScoreType::TrapScore1P, 10);
				FocusTrap->SetIsInstalled1P(true);
			}
			else
			{
				TrapperGameState->AddScore(EScoreType::TrapScore2P, 10);
				FocusTrap->SetIsInstalled1P(false);
			}

			if (!(FocusTrap->ActorHasTag("GunpowderBarrelTrap") ||
				FocusTrap->ActorHasTag("SpikeTrap")))
			{
				FocusTrap->SetOwner(Player);
			}
		}
	}
}

void UTrapMechanics::ServerRPCReInstallTrap_Implementation(ATrapBase* Trap)
{
	FocusTrap = Trap;
	MulticastRPCReInstallTrap();
}

void UTrapMechanics::MulticastRPCSnapTriggerSetTrapNullptr_Implementation(ATrapSnapTrigger* TSTrigger, ATrapBase* Trap)
{
	if (IsValid(TSTrigger))
	{
		//UE_LOG(LogTrap, Warning, TEXT("TSTrigger true"));
		TSTrigger->SetTrap(nullptr);
	}

	AGuillotinePendulumTrap* Guillotine = Cast<AGuillotinePendulumTrap>(Trap);
	if (IsValid(Guillotine))
	{
		Guillotine->DemolishGuillotine();
		Guillotine->SetMapmaker(false);
	}
}

void UTrapMechanics::MulticastRPCGuillotineSpawn_Implementation(ATrapSnapTrigger* TSTrigger)
{
	ATrapBase* SnapGuillotine = TSTrigger->GetSnapGuillotine();
	AGuillotinePendulumTrap* Guillotine = Cast<AGuillotinePendulumTrap>(SnapGuillotine);
	if (IsValid(Guillotine))
	{
		Guillotine->SetAllMaterialOriginal();
		Guillotine->SetActorEnableCollision(true);
		Guillotine->SetMapmaker(true);
		//Guillotine->SetInstallCost(TrapPartsTable[TrapPartsTableIndex].InstallCost);
		Guillotine->SetInstallCost(TrapDataArray[TrapPartsTableIndex].InstallCost);
	}
}

void UTrapMechanics::MulticastRPCStartInstallAnim_Implementation()
{
	Player->StartInstallAnim();
}

void UTrapMechanics::ServerRPCStartInstallAnim_Implementation()
{
	Player->StartInstallAnim();
}

void UTrapMechanics::MulticastRPCEndInstallAnim_Implementation()
{
	Player->EndInstallAnim();
}

void UTrapMechanics::ServerRPCEndInstallAnim_Implementation()
{
	Player->EndInstallAnim();
}

//void UTrapMechanics::MulticastRPCPlaySoundInstallTrap_Implementation()
//{
//	//UE_LOG(LogTrap, Warning, TEXT("MulticastRPCPlaySoundInstallTrap_Implementation"));
//	
//	// 사운드
//	if (Player->InteractionSound)
//	{
//		// 특정 지점에서 사운드 재생
//		UGameplayStatics::PlaySoundAtLocation(this, Player->InteractionSound, Player->GetActorLocation());
//	}
//}

void UTrapMechanics::ClientRPCPlaySoundInstallTrap_Implementation()
{
	PlaySoundInstallTrap();
}

void UTrapMechanics::SafeReleasePreviewMesh()
{
	/*if (!Player->HasAuthority())
	{
		UE_LOG(LogTrap, Warning, TEXT("SafeReleasePreviewMesh"));
	}*/

	if (PreviewMesh)
	{
		PreviewMesh->SetHiddenInGame(true);
		PreviewMesh->SetWorldLocation(FVector(0, 0, -100000));
	}

	if (PreviewRange)
	{
		PreviewRange->SetHiddenInGame(true);
		PreviewRange->SetWorldLocation(FVector(0, 0, -100000));
	}

	if (PreviewCouple)
	{
		PreviewCouple->SetHiddenInGame(true);
		PreviewCouple->SetWorldLocation(FVector(0, 0, -100000));
	}

	if (PreviewNiagara)
	{
		PreviewNiagara->SetActorHiddenInGame(true);
		PreviewNiagara->SetActorLocation(FVector(0, 0, -100000));
	}
	if (PreviewFirstCoupleMeshTemp)
	{
		PreviewFirstCoupleMeshTemp->SetHiddenInGame(true);
		PreviewFirstCoupleMeshTemp->SetWorldLocation(FVector(0, 0, -100000));
	}
	if (PreviewRangeDecal)
	{
		PreviewRangeDecal->SetActorHiddenInGame(true);
		PreviewRangeDecal->SetActorLocation(FVector(0, 0, -100000));
	}
}

void UTrapMechanics::MulticastRPCSafeReleasePreviewMesh_Implementation()
{
	if (PreviewMesh)
	{
		PreviewMesh->SetHiddenInGame(true);
		PreviewMesh->SetWorldLocation(FVector(0, 0, -100000));
	}

	if (PreviewRange)
	{
		PreviewRange->SetHiddenInGame(true);
		PreviewRange->SetWorldLocation(FVector(0, 0, -100000));
	}

	if (PreviewCouple)
	{
		PreviewCouple->SetHiddenInGame(true);
		PreviewCouple->SetWorldLocation(FVector(0, 0, -100000));
	}

	if (PreviewNiagara)
	{
		PreviewNiagara->SetActorHiddenInGame(true);
		PreviewNiagara->SetActorLocation(FVector(0, 0, -100000));
	}
	if (PreviewFirstCoupleMeshTemp)
	{
		PreviewFirstCoupleMeshTemp->SetHiddenInGame(true);
		PreviewFirstCoupleMeshTemp->SetWorldLocation(FVector(0, 0, -100000));
	}
	if (PreviewRangeDecal)
	{
		PreviewRangeDecal->SetActorHiddenInGame(true);
		PreviewRangeDecal->SetActorLocation(FVector(0, 0, -100000));
	}
}

//void UTrapMechanics::MulticastRPCFirstCoupleInstall_Implementation(ADecalActor* PRDecal)
//{
//	/*if (Player->HasAuthority())
//	{
//		if (IsValid(PRDecal))
//		{
//			UE_LOG(LogTrap, Warning, TEXT("Server PRDecal true"));
//			PRDecal->SetActorHiddenInGame(false);
//		}
//		else
//		{
//			UE_LOG(LogTrap, Warning, TEXT("Server PRDecal false"));
//		}
//	}
//	else
//	{
//		if (IsValid(PRDecal))
//		{
//			UE_LOG(LogTrap, Warning, TEXT("Client PRDecal true"));
//			PRDecal->SetActorHiddenInGame(false);
//		}
//		else
//		{
//			UE_LOG(LogTrap, Warning, TEXT("Client PRDecal false"));
//		}
//	}*/
//
//	if (IsValid(PRDecal))
//	{
//		PRDecal->SetActorHiddenInGame(false);
//	}
//
//	/*if (IsValid(PreviewRangeDecal))
//	{
//		PreviewRangeDecal->SetActorHiddenInGame(false);
//	}*/
//}

void UTrapMechanics::ServerRPCFirstCoupleInstall_Implementation()
{
	bCanInstallCouple = true;

	if (IsValid(PreviewRangeDecal))
	{
		PreviewRangeDecal->SetActorHiddenInGame(false);
	}

	PreviewFirstCoupleMeshTemp = PreviewMesh;
	SetPreviewMesh();
}

void UTrapMechanics::ServerRPCUpdateScoreHUD_Implementation()
{
	ATrapperGameState* GameState = GetWorld()->GetGameState<ATrapperGameState>();
	if (GameState)
	{
		GameState->UpdateScoreHUD();
	}
}

void UTrapMechanics::IncreaseTrapHudNum(ETrapType Type, bool bInstall)
{
	Player->ClientRPCUpdateTrapHudNum(Type, bInstall);
}

void UTrapMechanics::DecreaseTrapHudNum(ETrapType Type, bool bInstall)
{
	if (!IsValid(FocusTrap))
	{
		return;
	}

	/*bool bCanUpdate = true;
	if (Type == ETrapType::GunpowderBarrel)
	{
		if (FocusTrap->IsWeckage())
		{
			bCanUpdate = false;
		}
	}

	if (bCanUpdate)
	{
		ATrapperPlayer* TrapperPlayer = Cast<ATrapperPlayer>(FocusTrap->GetOwner());
		if (TrapperPlayer)
		{
			TrapperPlayer->ClientRPCUpdateTrapHudNum(Type, bInstall);
		}
	}*/

	ATrapperPlayer* TrapperPlayer = Cast<ATrapperPlayer>(FocusTrap->GetOwner());
	if (TrapperPlayer)
	{
		TrapperPlayer->ClientRPCUpdateTrapHudNum(Type, bInstall);
	}
}

//void UTrapMechanics::ShowMeTheMoney()
//{
//	if (!Player || !Player->IsLocallyControlled())
//	{
//		return;
//	}
//
//	if (Player->HasAuthority())
//	{
//		Player->AddBoneItem(Player->MaxItemCount);
//	}
//	else
//	{
//		Player->ServerRPCAddBoneItem(Player->MaxItemCount);
//	}
//}

void UTrapMechanics::ShowScoreHUD()
{
	if (!Player || !Player->IsLocallyControlled() || !PlayerController)
	{
		return;
	}

	ServerRPCUpdateScoreHUD();

	UScoreHUD* ScoreHUD = PlayerController->ScoreHudRef;
	if (ScoreHUD)
	{
		ESlateVisibility Visibility = ScoreHUD->GetVisibility();
		if (Visibility == ESlateVisibility::Visible)
		{
			ScoreHUD->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			ScoreHUD->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UTrapMechanics::SpawnMonsterSpikeTraps()
{
	if (!Player->IsLocallyControlled())
	{
		return;
	}

	FTransform CameraWorldTransform = FollowCamera->GetComponentTransform();
	FVector CameraLocation = CameraWorldTransform.GetLocation();
	FQuat CameraRotation = CameraWorldTransform.GetRotation();
	FVector CameraForwardVector = CameraRotation.GetForwardVector();
	FVector StartLocation = CameraLocation + CameraForwardVector * FVector::Distance(Player->GetActorLocation(), CameraLocation);
	FVector EndLocation = StartLocation + CameraForwardVector * MaxInstallRayDistance;

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());

	FHitResult HitResult;
	bool bHasHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, CollisionChannel, CollisionParams);

	if (bHasHit)
	{
		FTransform Transform;
		FVector Location = HitResult.ImpactPoint;
		Location.Z -= 25;
		Transform.SetLocation(Location);
		ServerRPCSpawnMonsterSpikeTraps(Transform);
	}
}

void UTrapMechanics::ServerRPCSpawnMonsterSpikeTraps_Implementation(FTransform Transform)
{
	TSubclassOf<ATrapBase> TrapActorClass;
	TrapActorClass = TrapPartsArray[0].ClassType;
	if (TrapActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Player;

		FVector CameraRightVector = Player->GetControlRotation().RotateVector(FVector::RightVector);
		CameraRightVector.Normalize();
		FVector Offset = CameraRightVector * 400.0f;

		ATrapBase* SpawnedTrap = GetWorld()->SpawnActor<ATrapBase>(TrapActorClass, Transform, SpawnParams);
		SpawnedTrap->SetDataTableInfo(
			0,
			100000,
			TrapDataArray[TrapPartsTableIndex].IncreaseDamage,
			TrapDataArray[TrapPartsTableIndex].Speed,
			TrapDataArray[TrapPartsTableIndex].IncreaseSpeed,
			TrapDataArray[TrapPartsTableIndex].Strength,
			TrapDataArray[TrapPartsTableIndex].IncreaseStrength,
			TrapDataArray[TrapPartsTableIndex].Duration);

		FTransform Transform1 = Transform;
		Transform1.SetLocation(Transform.GetLocation() - Offset);

		ATrapBase* SpawnedTrap1 = GetWorld()->SpawnActor<ATrapBase>(TrapActorClass, Transform1, SpawnParams);
		SpawnedTrap1->SetDataTableInfo(
			0,
			100000,
			TrapDataArray[TrapPartsTableIndex].IncreaseDamage,
			TrapDataArray[TrapPartsTableIndex].Speed,
			TrapDataArray[TrapPartsTableIndex].IncreaseSpeed,
			TrapDataArray[TrapPartsTableIndex].Strength,
			TrapDataArray[TrapPartsTableIndex].IncreaseStrength,
			TrapDataArray[TrapPartsTableIndex].Duration);

		FTransform Transform2 = Transform;
		Transform2.SetLocation(Transform.GetLocation() + Offset);

		ATrapBase* SpawnedTrap2 = GetWorld()->SpawnActor<ATrapBase>(TrapActorClass, Transform2, SpawnParams);
		SpawnedTrap2->SetDataTableInfo(
			0,
			100000,
			TrapDataArray[TrapPartsTableIndex].IncreaseDamage,
			TrapDataArray[TrapPartsTableIndex].Speed,
			TrapDataArray[TrapPartsTableIndex].IncreaseSpeed,
			TrapDataArray[TrapPartsTableIndex].Strength,
			TrapDataArray[TrapPartsTableIndex].IncreaseStrength,
			TrapDataArray[TrapPartsTableIndex].Duration);
	}
}

//void UTrapMechanics::ServerRPCChangePreviewMesh_Implementation()
	//{
	//	PermissionChangePreviewMesh();
	//}



	//PreviewMesh->SetStaticMesh(nullptr);
	//PreviewMesh->UnregisterComponent();
	//PreviewMesh->DestroyComponent();
	//
	////PreviewMesh->DestroyComponent();
	//
	//// PreviewMeshTransform �� Rotation �ʱ�ȭ
	//PreviewMeshTransform.SetIdentity();
	//PreviewMeshRotator = FRotator(0, 0, 0);
	//PreviewMeshRotationYaw = 0.f;
	//
	//if (IsValid(PreviewRange))
	//{
	//	PreviewRange->SetStaticMesh(nullptr);
	//	PreviewRange->UnregisterComponent();
	//	PreviewRange->DestroyComponent();
	//	PreviewRange = nullptr;
	//}
	//
	//if (IsValid(PreviewNiagara))
	//{
	//	PreviewNiagara->Destroy();
	//}
	//
	///*if (IsValid(PreviewCouple))
	//{
	//	PreviewCouple->SetStaticMesh(nullptr);
	//	PreviewCouple->UnregisterComponent();
	//	PreviewCouple->DestroyComponent();
	//	PreviewCouple = nullptr;
	//
	//	PreviewCoupleTransform.SetIdentity();
	//	PreviewMeshRotator = FRotator(0, 0, 0);
	//	PreviewMeshRotationYaw = 0.f;
	//}*/