#pragma once

#include "CoreMinimal.h"

UENUM()
enum class ETrapType : uint8
{
	Spike = 0,				// 가시
	Bear,					// 곰
	Plank,					// 판자
	GuillotinePendulum,		// 길로틴
	GunpowderBarrel,		// 화약통
	OilBag,					// 기름주머니
	Clap,					// 박수
	Spear,					// 말뚝?

	None,
};


UENUM(BlueprintType)
enum class ETrapInstallLocation : uint8
{
	E_Floor				UMETA(DisplayName = "Floor"),
	E_Wall				UMETA(DisplayName = "Wall"),
	E_Ceiling			UMETA(DisplayName = "Ceiling"),
};


UENUM(BlueprintType)
enum class ETrapActivationMethod : uint8
{
	E_DefaultArrow		 UMETA(DisplayName = "DefaultArrow"),
	E_MagneticArrow		 UMETA(DisplayName = "MagneticArrow"),
	E_MagneticArrowMulti UMETA(DisplayName = "MagneticArrowMulti"),
	E_Auto				 UMETA(DisplayName = "Auto"),
};

UENUM(BlueprintType)
enum class ETrapCCType : uint8
{
	E_Down				UMETA(DisplayName = "Down"),
	E_KnockBack			UMETA(DisplayName = "NnockBack"),
	E_Stun				UMETA(DisplayName = "Stun"),
	E_Slow				UMETA(DisplayName = "Slow"),
	E_None				UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
enum class ETrapHUDType : uint8
{
	E_Unhovered			UMETA(DisplayName = "Unhovered"),
	E_Enabled			UMETA(DisplayName = "Enabled"),
	E_Disabled			UMETA(DisplayName = "Disabled"),
};