
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AimTargetInterface.generated.h"

UINTERFACE(BlueprintType)
class UAimTargetInterface : public UInterface
{
    GENERATED_BODY()
};

class IAutoAimTargetInterface
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Aim Target")
    FVector GetAimTargetLocation() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Aim Target")
    bool CanBeAimTarget() const;
};
