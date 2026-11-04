
#include "Components/AutoAimAssistComponent.h"
#include "Weapon/WeaponBase.h"
#include "Interfaces/AimTargetInterface.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "CollisionQueryParams.h"
#include "CollisionObjectQueryParams.h"
#include "Math/RotationMatrix.h"

UAutoAimAssistComponent::UAutoAimAssistComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    bAutoActivate = true;
}

void UAutoAimAssistComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeOwner();
}

bool UAutoAimAssistComponent::InitializeOwner()
{
    Weapon = Cast<AWeaponBase>(GetOwner());

    if (!Weapon.IsValid())
        return false;

    OwningPawn = Cast<APawn>(Weapon->GetOwner());

    if (!OwningPawn.IsValid())
        return false;

    PlayerController = Cast<APlayerController>(
        OwningPawn->GetController()
    );

    if (!PlayerController.IsValid())
        return false;

    return true;
}



bool UAutoAimAssistComponent::IsValidTarget(AActor* Target) const
{
    if (!IsValid(Target))
        return false;

    if (Target == Weapon.Get() || Target == OwningPawn.Get())
        return false;

    if (!Target->GetClass()->ImplementsInterface(UAutoAimTargetInterface::StaticClass()))
    {
        return false;
    }

    if (!IAutoAimTargetInterface::Execute_CanBeAimTarget(Target))
        return false;

    return true;
}

bool UAutoAimAssistComponent::GetTargetLocation( AActor* Target,  FVector& OutLocation) const
{
    if (!IsValidTarget(Target))
        return false;

    OutLocation =IAutoAimTargetInterface::Execute_GetAimTargetLocation(Target);

    return !OutLocation.ContainsNaN();
}



bool UAutoAimAssistComponent::HasLineOfSight(AActor* Target,const FVector& ViewLocation,const FVector& TargetLocation) const
{
    if (!bRequireLineOfSight)
        return true;

    if (!GetWorld() || !IsValid(Target))
        return false;

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AimAssistVisibility),true);

    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(OwningPawn.Get());

    FHitResult HitResult;

    const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        ViewLocation,
        TargetLocation,
        ECC_Visibility,
        QueryParams
    );

    if (!bBlocked)
        return true;

    return HitResult.GetActor() == Target;
}


AActor* UAutoAimAssistComponent::FindBestTarget(const FVector& ViewLocation,const FRotator& ViewRotation,AActor* PreviousTarget) const
{
    if (!GetWorld() || !PlayerController.IsValid())
        return nullptr;

    FVector2D ViewportSize;

    PlayerController->GetViewportSize(ViewportSize);

    if (ViewportSize.IsNearlyZero())
        return nullptr;

    const FVector2D ScreenCenter = ViewportSize * 0.5f;
    const FVector ViewDirection = ViewRotation.Vector();

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AimAssistDetection),false);

    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(OwningPawn.Get());

    TArray<FOverlapResult> Overlaps;

    const bool bFoundOverlaps = GetWorld()->OverlapMultiByObjectType
    (
        Overlaps,
        ViewLocation,
        FQuat::Identity,
        ObjectParams,
        FCollisionShape::MakeSphere(DetectionRadius),
        QueryParams
    );

    if (!bFoundOverlaps)
        return nullptr;

    AActor* BestTarget = nullptr;

    float BestScore = TNumericLimits<float>::Max();
    float PreviousTargetScore = TNumericLimits<float>::Max();

    const float MinimumDot = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(MaximumAimAngle, 0.0f, 179.0f)));

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Candidate = Overlap.GetActor();

        if (!IsValidTarget(Candidate))
            continue;

        FVector TargetLocation;

        if (!GetTargetLocation(Candidate, TargetLocation))
            continue;

        const FVector ToTarget = TargetLocation - ViewLocation;

        const float DistanceSquared = ToTarget.SizeSquared();

        if (DistanceSquared > FMath::Square(MaximumTargetDistance))
            continue;

        if (DistanceSquared <= UE_SMALL_NUMBER)
            continue;

        const FVector Direction = ToTarget.GetSafeNormal();

        const float Dot = FVector::DotProduct(ViewDirection,Direction);

        if (Dot < MinimumDot)
            continue;

        if (!HasLineOfSight(
            Candidate,
            ViewLocation,
            TargetLocation))
        {
            continue;
        }

        const float Score = CalculateTargetScore
        (
            ViewLocation,
            ViewDirection,
            TargetLocation,
            ScreenCenter,
            MaximumTargetDistance
        );

        if (Score >= TNumericLimits<float>::Max())
            continue;

        if (Candidate == PreviousTarget)
        {
            PreviousTargetScore = Score;
        }

        if (Score < BestScore)
        {
            BestScore = Score;
            BestTarget = Candidate;
        }
    }

    if (IsValid(PreviousTarget) &&
        PreviousTargetScore < TNumericLimits<float>::Max() &&
        BestTarget != PreviousTarget &&
        BestScore + TargetSwitchThreshold >= PreviousTargetScore)
    {
        return PreviousTarget;
    }

    return BestTarget;
}



void UAutoAimAssistComponent::UpdateAimRotation(float DeltaTime,const FVector& TargetLocation)
{
    if (!PlayerController.IsValid()) return;

    if (DeltaTime <= 0.0f) return;

    FVector ViewLocation;
    FRotator CurrentRotation;

    PlayerController->GetPlayerViewPoint(ViewLocation,CurrentRotation);

    const FVector Direction = TargetLocation - ViewLocation;

    if (Direction.IsNearlyZero())
        return;

    const FRotator DesiredRotation = Direction.Rotation();

    const float EffectiveStrength = FMath::Clamp(AimStrength, MinimumAimStrength, 1.0f);

    const float Alpha = 1.0f - FMath::Exp(-FMath::Max(AimSmoothingSpeed, 0.0f) * DeltaTime);

    const float FinalAlpha = FMath::Clamp(Alpha * EffectiveStrength,0.0f,1.0f);

    const FQuat CurrentQuaternion = CurrentRotation.Quaternion();

    const FQuat TargetQuaternion = DesiredRotation.Quaternion();

    const FQuat NewQuaternion = FQuat::Slerp
    (
        CurrentQuaternion,
        TargetQuaternion,
        FinalAlpha
    ).GetNormalized();

    PlayerController->SetControlRotation(
        NewQuaternion.Rotator()
    );
}

void UAutoAimAssistComponent::UpdateAim(
    float DeltaTime,
    const FVector& TargetLocation)
{
    UpdateAimRotation(DeltaTime, TargetLocation);
}



void UAutoAimAssistComponent::UpdateTarget(float DeltaTime)
{
    if (!bAimAssistEnabled)
        return;

    if (!PlayerController.IsValid() || !OwningPawn.IsValid())
    {
        return;
    }

    FVector ViewLocation;
    FRotator ViewRotation;

    PlayerController->GetPlayerViewPoint(ViewLocation,ViewRotation);

    AActor* PreviousTarget = CurrentTarget.Get();

    AActor* NewTarget = FindBestTarget
    (
        ViewLocation,
        ViewRotation,
        PreviousTarget
    );

    if (NewTarget)
    {
        CurrentTarget = NewTarget;
        TargetLostTime = 0.0f;

        FVector TargetLocation;

        if (GetTargetLocation(NewTarget, TargetLocation))
        {
            UpdateAim(DeltaTime, TargetLocation);
        }

        return;
    }

    if (bMaintainTarget && IsValidTarget(PreviousTarget))
    {
        FVector TargetLocation;

        const bool bHasLocation =GetTargetLocation(PreviousTarget, TargetLocation);

        const bool bWithinRange = bHasLocation &&
            FVector::DistSquared(ViewLocation, TargetLocation) <=
            FMath::Square(MaximumTargetDistance);

        const bool bVisible =
            bHasLocation &&
            HasLineOfSight(
                PreviousTarget,
                ViewLocation,
                TargetLocation
            );

        if (bWithinRange && bVisible)
        {
            TargetLostTime = 0.0f;
            UpdateAim(DeltaTime, TargetLocation);
            return;
        }
    }

    TargetLostTime += DeltaTime;

    if (TargetLostTime >= TargetReleaseGracePeriod)
    {
        ClearTarget();
    }
}


void UAutoAimAssistComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(
        DeltaTime,
        TickType,
        ThisTickFunction
    );

    if (!bAimAssistEnabled || !GetOwner())
        return;

    if (!Weapon.IsValid() ||
        !OwningPawn.IsValid() ||
        !PlayerController.IsValid())
    {
        if (!InitializeOwner())
            return;
    }

    if (!OwningPawn->IsLocallyControlled())
        return;

    UpdateTarget(DeltaTime);
}

void UAutoAimAssistComponent::SetAimAssistEnabled(bool bEnabled)
{
    bAimAssistEnabled = bEnabled;

    if (!bEnabled)
    {
        ClearTarget();
    }
}

void UAutoAimAssistComponent::ClearTarget()
{
    CurrentTarget.Reset();
    TargetLostTime = 0.0f;
}

AActor* UAutoAimAssistComponent::GetCurrentTarget() const
{
    return CurrentTarget.Get();
}

bool UAutoAimAssistComponent::HasTarget() const
{
    return CurrentTarget.IsValid();
}




