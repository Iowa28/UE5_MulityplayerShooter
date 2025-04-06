// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MultiplayerShooter/HUD/BaseHUD.h"
#include "MultiplayerShooter/Types/CombatState.h"
#include "MultiplayerShooter/Types/WeaponTypes.h"
#include "CombatComponent.generated.h"

UCLASS( ClassGroup = (Custom), meta = (BlueprintSpawnableComponent, PrioritizeCategories ="Combat Ammo") )
class MULTIPLAYERSHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

	friend class ABaseCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void FireButtonPressed(bool bPressed);

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SwapWeapons();

	bool bLocallyReloading;

	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();

	UFUNCTION(BlueprintCallable)
	void ShotgunShotReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_PrimaryWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	UFUNCTION()
	void OnRep_Flag();

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	void EquipFlag(AWeapon* Flag);

	void DropEquippedWeapon();
	void AttachWeaponToRightHand(AWeapon* WeaponToAttach);
	void AttachWeaponToLeftHand(AWeapon* WeaponToAttach);
	void AttachFlagToLeftHand(AWeapon* Flag);
	void AttachWeaponToBackpack(AWeapon* WeaponToAttach);
	void PlayEquippedWeaponSound(AWeapon* Weapon);
	void UpdateCarriedAmmo();
	void ReloadEmptyWeapon();
	
	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class AProjectile> GrenadeClass;

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	void ShowAttachedGrenade(bool bShowGrenade);

private:
	UPROPERTY()
	ABaseCharacter* Character;
	
	UPROPERTY()
	class ABasePlayerController* Controller;
	
	UPROPERTY()
	ABaseHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_PrimaryWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Flag)
	AWeapon* TheFlag;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming;

	bool bAimButtonPressed;

	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AimWalkSpeed;
	
	float BaseWalkSpeed;

	bool bFireButtonPressed;

	FVector HitTarget;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootFactor;

	FHUDPackage HUDPackage;

	float DefaultFOV;
	float CurrentFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	bool bCanFire = true;

	FTimerHandle FireTimer;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire() const;

	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MaxCarriedAmmo = 500;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 StartingARAmmo = 30;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 StartingPistolAmmo = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 StartingSMGAmmo = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 StartingShotgunAmmo = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 StartingSniperAmmo = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 StartingGrenadeLauncherAmmo = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MaxGrenades = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	void UpdateHUDGrenades();

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	int32 AmountToReload();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

	UPROPERTY(ReplicatedUsing = OnRep_HoldingTheFlag)
	bool bHoldingTheFlag;

	UFUNCTION()
	void OnRep_HoldingTheFlag();

public:
	FORCEINLINE int32 GetGrenades() const { return Grenades; }

	bool ShouldSwapWeapons() const;
};
