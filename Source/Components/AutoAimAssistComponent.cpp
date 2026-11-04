
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
