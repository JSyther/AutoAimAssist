
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoAimAssistComponent.generated.h"

class AActor;
class APawn;
class APlayerController;
class AWeaponBase;

UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class AUTOAIMASSIST_API UAutoAimAssistComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UAutoAimAssistComponent();

    virtual void BeginPlay() override;

    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Aim Assist")
    void SetAimAssistEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Aim Assist")
    void ClearTarget();

    UFUNCTION(BlueprintPure, Category = "Aim Assist")
    AActor* GetCurrentTarget() const;

    UFUNCTION(BlueprintPure, Category = "Aim Assist")
    bool HasTarget() const;

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|General")
    bool bAimAssistEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|Detection")
    float DetectionRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|Detection")
    float MaximumTargetDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|Detection")
    float MaximumAimAngle = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|Detection")
    float MaximumScreenDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|Detection")
    float TargetSwitchThreshold = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|Smoothing")
    float AimSmoothingSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|Smoothing")
    float AimStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|Smoothing")
    float MinimumAimStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|Targeting")
    bool bRequireLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|Targeting")
    bool bMaintainTarget = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim Assist|Targeting")
    float TargetReleaseGracePeriod = 0.2f;

private:

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> CurrentTarget;

    TWeakObjectPtr<AWeaponBase> Weapon;
    TWeakObjectPtr<APawn> OwningPawn;
    TWeakObjectPtr<APlayerController> PlayerController;

    float TargetLostTime = 0.0f;

    bool InitializeOwner();

    AActor* FindBestTarget(
        const FVector& ViewLocation,
        const FRotator& ViewRotation,
        AActor* PreviousTarget) const;

    bool IsValidTarget(AActor* Target) const;

    bool HasLineOfSight(
        AActor* Target,
        const FVector& ViewLocation,
        const FVector& TargetLocation) const;

    bool GetTargetLocation(
        AActor* Target,
        FVector& OutLocation) const;

    float CalculateTargetScore(
        const FVector& ViewLocation,
        const FVector& ViewDirection,
        const FVector& TargetLocation,
        const FVector2D& ScreenCenter,
        float MaximumDistance) const;

    void UpdateAim(
        float DeltaTime,
        const FVector& TargetLocation);

    void UpdateTarget(float DeltaTime);

    void UpdateAimRotation(
        float DeltaTime,
        const FVector& TargetLocation);
};
