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

	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void ShotgunShotReload();

	void JumpToShotgunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void DropEquippedWeapon();
	void AttachWeaponToRightHand(AWeapon* WeaponToAttach);
	void AttachWeaponToLeftHand(AWeapon* WeaponToAttach);
	void UpdateCarriedAmmo();
	void PlayEquippedWeaponSound();
	void ReloadEmptyWeapon();
	
	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

private:
	UPROPERTY()
	ABaseCharacter* Character;
	
	UPROPERTY()
	class ABasePlayerController* Controller;
	
	UPROPERTY()
	ABaseHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

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
	float ZoomedFOV = 30.f;

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

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	int32 AmountToReload();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
};
