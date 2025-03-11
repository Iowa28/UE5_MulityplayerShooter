// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MultiplayerShooter/Types/Team.h"
#include "MultiplayerShooter/Types/WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class MULTIPLAYERSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void ShowPickupWidget(bool bShowWidget);

	virtual void Fire(const FVector& HitTarget);

	virtual void Dropped();

	void SetHUDAmmo();

	void AddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditDefaultsOnly)
	class USoundCue* EquipSound;

	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

	UPROPERTY(EditDefaultsOnly)
	EFireType FireType;

	UPROPERTY(EditDefaultsOnly, Category = "Scatter")
	bool bUseScatter = false;

	FVector TraceEndWithScatter(const FVector& HitTarget);

protected:
	virtual void BeginPlay() override;

	virtual void OnRep_Owner() override;

	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnEquippedSecondary();
	virtual void OnDropped();

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UPROPERTY(EditDefaultsOnly, Category = "Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditDefaultsOnly)
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly)
	float HeadShotDamage = 40.f;

	UPROPERTY(EditAnywhere, Replicated)
	bool bUseServerSideRewind = false;
	
	UPROPERTY()
	class ABaseCharacter* OwnerCharacter;

	UPROPERTY()
	class ABasePlayerController* OwnerController;

	UFUNCTION()
	void OnHighPing(bool bHighPing);

private:
	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly)
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere)
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditDefaultsOnly)
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditDefaultsOnly)
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditDefaultsOnly)
	int32 Ammo;

	// The number of unprocessed server requests for Ammo
	int32 Sequence = 0;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	void SpendRound();

	UPROPERTY(EditDefaultsOnly)
	int32 MagCapacity;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairCenter;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairRight;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairTop;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairBottom;

	UPROPERTY(EditDefaultsOnly)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditDefaultsOnly)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditDefaultsOnly)
	float FireDelay = .15f;

	UPROPERTY(EditDefaultsOnly)
	bool bAutomatic = true;

	UPROPERTY(EditDefaultsOnly)
	EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly)
	ETeam Team;

public:
	void SetWeaponState(EWeaponState State);

	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	
	FORCEINLINE UTexture2D* GetCrosshairCenter() const { return CrosshairCenter; }
	FORCEINLINE UTexture2D* GetCrosshairLeft() const { return CrosshairLeft; }
	FORCEINLINE UTexture2D* GetCrosshairRight() const { return CrosshairRight; }
	FORCEINLINE UTexture2D* GetCrosshairTop() const { return CrosshairTop; }
	FORCEINLINE UTexture2D* GetCrosshairBottom() const { return CrosshairBottom; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	
	FORCEINLINE float GetFireDelay() const { return FireDelay; }
	FORCEINLINE bool IsAutomatic() const { return bAutomatic; }
	
	FORCEINLINE bool IsEmpty() const { return Ammo <= 0; }
	FORCEINLINE bool IsFull() const { return Ammo == MagCapacity; }

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE ETeam GetTeam() const { return Team; }

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
};
