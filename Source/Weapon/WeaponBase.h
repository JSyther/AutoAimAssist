
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "WeaponBase.generated.h"

class USkeletalMeshComponent;
class UAutoAimAssistComponent;
class AController;

UCLASS()
class AUTOAIMASSIST_API AWeaponBase : public AActor
{
    GENERATED_BODY()

public:

    AWeaponBase();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void StartFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void StopFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Fire();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Reload();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void SetWeaponEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Weapon")
    int32 GetCurrentAmmo() const;

    UFUNCTION(BlueprintPure, Category = "Weapon")
    int32 GetReserveAmmo() const;

    UFUNCTION(BlueprintPure, Category = "Weapon")
    EWeaponState GetWeaponState() const;

    UFUNCTION(BlueprintPure, Category = "Weapon")
    USkeletalMeshComponent* GetWeaponMesh() const;

    UFUNCTION(BlueprintPure, Category = "Weapon")
    UAutoAimAssistComponent* GetAimAssistComponent() const;

protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    TObjectPtr<USkeletalMeshComponent> WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    TObjectPtr<UAutoAimAssistComponent> AimAssistComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Configuration")
    EWeaponFireMode FireMode = EWeaponFireMode::SemiAutomatic;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Configuration")
    float FireRate = 0.12f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Configuration")
    float WeaponDamage = 25.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Configuration")
    float HeadshotMultiplier = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Configuration")
    float MaximumRange = 15000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Configuration")
    FName MuzzleSocketName = TEXT("Muzzle");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammunition")
    int32 MagazineCapacity = 30;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammunition")
    int32 InitialReserveAmmo = 120;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Ammunition")
    int32 CurrentAmmo = 30;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|Ammunition")
    int32 ReserveAmmo = 120;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon|State")
    EWeaponState WeaponState = EWeaponState::Ready;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Reload")
    float ReloadDuration = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Damage")
    TSubclassOf<UDamageType> DamageTypeClass;

private:

    FTimerHandle FireTimerHandle;
    FTimerHandle ReloadTimerHandle;

    float LastFireTime = -1.0f;

    bool bWantsToFire = false;

    bool CanFire() const;

    void PerformFire();

    void FinishReload();

    void ScheduleNextShot();

    AController* GetWeaponController() const;
};
