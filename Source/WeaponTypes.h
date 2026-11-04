
#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    Ready,
    Firing,
    Reloading,
    Disabled
};

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
    SemiAutomatic,
    Automatic
};
