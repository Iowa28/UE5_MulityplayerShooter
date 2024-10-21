// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MultiplayerShooter/Types/WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
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

	void Dropped();

	void SetHUDAmmo();

	void AddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	class USoundCue* EquipSound;

	void EnableCustomDepth(bool bEnable);

	bool bDestroyWeapon = false;

protected:
	virtual void BeginPlay() override;

	virtual void OnRep_Owner() override;

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

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_Ammo, Category = "Weapon")
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 MagCapacity;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UTexture2D* CrosshairCenter;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UTexture2D* CrosshairRight;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UTexture2D* CrosshairTop;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UTexture2D* CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float FireDelay = .15f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	bool bAutomatic = true;

	UPROPERTY()
	class ABaseCharacter* OwnerCharacter;

	UPROPERTY()
	class ABasePlayerController* OwnerController;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	EWeaponType WeaponType;

public:
	void SetWeaponState(EWeaponState State);

	FORCEINLINE USphereComponent* GerAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	
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

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
