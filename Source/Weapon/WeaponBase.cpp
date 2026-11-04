
#include "WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AutoAimAssistComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AWeaponBase::AWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);

    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    AimAssistComponent = CreateDefaultSubobject<UAutoAimAssistComponent>(TEXT("AimAssist"));
}

void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    CurrentAmmo = FMath::Clamp(MagazineCapacity, 0, MagazineCapacity);
    ReserveAmmo = FMath::Max(InitialReserveAmmo, 0);

    if (!DamageTypeClass)
    {
        DamageTypeClass = UDamageType::StaticClass();
    }
}

bool AWeaponBase::CanFire() const
{
    return WeaponState == EWeaponState::Ready &&
        CurrentAmmo > 0 &&
        GetOwner() != nullptr &&
        GetWeaponController() != nullptr;
}

AController* AWeaponBase::GetWeaponController() const
{
    APawn* WeaponOwner = Cast<APawn>(GetOwner());

    return WeaponOwner ? WeaponOwner->GetController() : nullptr;
}

void AWeaponBase::StartFire()
{
    if (WeaponState == EWeaponState::Disabled)
        return;

    bWantsToFire = true;

    if (FireMode == EWeaponFireMode::Automatic)
    {
        Fire();
    }
    else
    {
        Fire();
    }
}

void AWeaponBase::StopFire()
{
    bWantsToFire = false;

    GetWorldTimerManager().ClearTimer(FireTimerHandle);

    if (WeaponState == EWeaponState::Firing)
    {
        WeaponState = EWeaponState::Ready;
    }
}

void AWeaponBase::Fire()
{
    if (!CanFire())
        return;

    const float CurrentTime = GetWorld()->GetTimeSeconds();

    if (LastFireTime >= 0.0f &&
        CurrentTime - LastFireTime < FireRate)
    {
        if (FireMode == EWeaponFireMode::Automatic && bWantsToFire)
        {
            ScheduleNextShot();
        }

        return;
    }

    PerformFire();
}

void AWeaponBase::PerformFire()
{
    AController* Controller = GetWeaponController();

    if (!Controller || CurrentAmmo <= 0)
        return;

    FVector ViewLocation;
    FRotator ViewRotation;

    Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

    const FVector TraceStart = ViewLocation;
    const FVector TraceEnd = TraceStart + ViewRotation.Vector() * MaximumRange;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;

    FHitResult HitResult;

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    --CurrentAmmo;

    LastFireTime = GetWorld()->GetTimeSeconds();

    WeaponState = EWeaponState::Firing;

    if (bHit && HitResult.GetActor())
    {
        float FinalDamage = WeaponDamage;

        if (HitResult.BoneName == TEXT("head"))
        {
            FinalDamage *= HeadshotMultiplier;
        }

        UGameplayStatics::ApplyPointDamage(
            HitResult.GetActor(),
            FinalDamage,
            ViewRotation.Vector(),
            HitResult,
            Controller,
            this,
            DamageTypeClass
        );
    }

    if (FireMode == EWeaponFireMode::Automatic && bWantsToFire)
    {
        ScheduleNextShot();
    }
    else
    {
        WeaponState = EWeaponState::Ready;
    }
}

void AWeaponBase::ScheduleNextShot()
{
    if (!GetWorld() || !bWantsToFire)
        return;

    const float Elapsed = GetWorld()->GetTimeSeconds() - LastFireTime;
    const float Remaining = FMath::Max(0.0f, FireRate - Elapsed);

    GetWorldTimerManager().SetTimer(
        FireTimerHandle,
        this,
        &AWeaponBase::Fire,
        Remaining,
        false
    );
}

void AWeaponBase::Reload()
{
    if (WeaponState == EWeaponState::Disabled ||
        WeaponState == EWeaponState::Reloading ||
        CurrentAmmo >= MagazineCapacity ||
        ReserveAmmo <= 0)
    {
        return;
    }

    StopFire();

    WeaponState = EWeaponState::Reloading;

    GetWorldTimerManager().SetTimer(
        ReloadTimerHandle,
        this,
        &AWeaponBase::FinishReload,
        ReloadDuration,
        false
    );
}

void AWeaponBase::FinishReload()
{
    if (WeaponState != EWeaponState::Reloading)
        return;

    const int32 RequiredAmmo = MagazineCapacity - CurrentAmmo;
    const int32 AmmoToLoad = FMath::Min(RequiredAmmo, ReserveAmmo);

    CurrentAmmo += AmmoToLoad;
    ReserveAmmo -= AmmoToLoad;

    WeaponState = EWeaponState::Ready;
}

void AWeaponBase::SetWeaponEnabled(bool bEnabled)
{
    if (!bEnabled)
    {
        StopFire();

        GetWorldTimerManager().ClearTimer(ReloadTimerHandle);

        WeaponState = EWeaponState::Disabled;
        AimAssistComponent->SetComponentTickEnabled(false);
    }
    else if (WeaponState == EWeaponState::Disabled)
    {
        WeaponState = EWeaponState::Ready;
        AimAssistComponent->SetComponentTickEnabled(true);
    }
}

int32 AWeaponBase::GetCurrentAmmo() const
{
    return CurrentAmmo;
}

int32 AWeaponBase::GetReserveAmmo() const
{
    return ReserveAmmo;
}

EWeaponState AWeaponBase::GetWeaponState() const
{
    return WeaponState;
}

USkeletalMeshComponent* AWeaponBase::GetWeaponMesh() const
{
    return WeaponMesh;
}

UAutoAimAssistComponent* AWeaponBase::GetAimAssistComponent() const
{
    return AimAssistComponent;
}
