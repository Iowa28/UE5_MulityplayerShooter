// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "MultiplayerShooter/Interfaces/CrosshairInteractInterface.h"
#include "MultiplayerShooter/Types/CombatState.h"
#include "MultiplayerShooter/Types/Team.h"
#include "MultiplayerShooter/Types/TurningInPlace.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS(meta = (PrioritizeCategories ="Elimination Combat"))
class MULTIPLAYERSHOOTER_API ABaseCharacter : public ACharacter, public ICrosshairInteractInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;

	virtual void Jump() override;

	/**
	 * Play montages
	 */
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayEliminationMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapWeaponMontage();
	
	virtual void OnRep_ReplicatedMovement() override;

	UFUNCTION()
	void Eliminate(bool bPlayerLeftGame);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate(bool bPlayerLeftGame);
	
	void DropOrDestroyWeapon(AWeapon* Weapon);

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ToggleSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();
	void UpdateHUDShield();

	void SpawnDefaultWeapon();

	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	bool bFinishedSwapping = true;

	FOnLeftGame OnLeftGame;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	void SetTeamColor(const ETeam Team);

protected:
	virtual void BeginPlay() override;
	
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void EquipButtonPressed();
	void SwapButtonPressed();
	
	void DuckButtonPressed();
	void DuckButtonReleased();

	void AimButtonPressed();
	void AimButtonReleased();

	void RotateInPlace(float DeltaTime);
	void AimOffset(float DeltaTime);
	void CalculateAimOffsetPitch();
	void SimProxiesTurn();

	void FireButtonPressed();
	void FireButtonReleased();

	void ReloadButtonPressed();
	
	void ThrowGrenadeButtonPressed();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	void PollInit();

	/**
	 * Hit boxes used for server-side rewind
	 */
#pragma region HitBoxes
	
	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Head;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Pelvis;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Spine_02;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Spine_03;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* UpperArm_L;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* UpperArm_R;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* LowerArm_L;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* LowerArm_R;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Hand_L;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Hand_R;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Backpack;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Blanket;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Thigh_L;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Thigh_R;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Calf_L;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Calf_R;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Foot_L;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* Foot_R;

#pragma endregion HitBoxes

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* FollowCamera;

#pragma region Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* DuckAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* ThrowGrenadeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SwapAction;
	
#pragma endregion Actions

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	class UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* BuffComponent;

	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensationComponent;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerSwapButtonPressed();

	float AimOffsetYaw;
	float InterpAimOffsetYaw;
	float AimOffsetPitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	void TurnInPlace(float DeltaTime);

	/**
	 * Animation Montages
	 */
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* SwapWeaponsMontage;

	void PlayHitReactMontage();

	UPROPERTY(EditDefaultsOnly)
	float CameraThreshold = 200.f;
	
	void HideCharacterIfCameraIsClose();

	bool bRotateRootBone;
	float TurnThreshold = 1.f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	float CalculateSpeed() const;

	UPROPERTY()
	class ABasePlayerController* BasePlayerController;

#pragma region Health
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health, Category = "Player Stats")
	float Health;

	bool bEliminated;

	UFUNCTION()
	void OnRep_Health(float LastHealth);
#pragma endregion Health

#pragma region Shield
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Shield, Category = "Player Stats")
	float Shield;

	UFUNCTION()
	void OnRep_Shield(float LastShield);
#pragma endregion Shield

#pragma region Elimination
	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	UAnimMontage* EliminationMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	float EliminationDelay = 3.f;
	FTimerHandle EliminationTimer;

	void EliminationTimerFinished();

	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	UCurveFloat* DissolveCurve;

	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UMaterialInstance* DissolveMaterialInstance;
	
	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	UMaterialInstance* DefaultMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	UMaterialInstance* RedMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	UMaterialInstance* RedDissolveMaterialInstance;

	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	UMaterialInstance* BlueMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	UMaterialInstance* BlueDissolveMaterialInstance;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	UParticleSystem* EliminationEffect;

	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UParticleSystemComponent* EliminationComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	class USoundCue* EliminationSound;

	bool bLeftGame = false;
#pragma endregion Elimination

	UPROPERTY()
	class ABasePlayerState* CharacterPlayerState;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	class UNiagaraSystem* CrownSystem;

	UPROPERTY()
	class UNiagaraComponent* CrownComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	void UpdateHUDAmmo();

	UPROPERTY()
	class AShooterGameMode* GameMode;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped() const;
	
	bool IsAiming() const;

	FORCEINLINE float GetAimOffsetYaw() const { return AimOffsetYaw; }
	FORCEINLINE float GetAimOffsetPitch() const { return AimOffsetPitch; }

	AWeapon* GetEquippedWeapon() const;

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	
	FORCEINLINE bool IsEliminated() const { return bEliminated; }
	
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(const float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(const float Amount) { Shield = Amount; }

	ECombatState GetCombatState() const;

	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensationComponent; }
	
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }

	bool IsLocallyReloading();
};
