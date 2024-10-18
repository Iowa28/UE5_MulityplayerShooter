// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "MultiplayerShooter/Interfaces/CrosshairInteractInterface.h"
#include "MultiplayerShooter/Types/CombatState.h"
#include "MultiplayerShooter/Types/TurningInPlace.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS(meta = (PrioritizeCategories ="Ammo Combat Input Elimination"))
class MULTIPLAYERSHOOTER_API ABaseCharacter : public ACharacter, public ICrosshairInteractInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void PostInitializeComponents() override;

	virtual void Jump() override;

	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayEliminationMontage();
	void PlayThrowGrenadeMontage();
	
	virtual void OnRep_ReplicatedMovement() override;

	UFUNCTION()
	void Eliminate();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate();

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ToggleSniperScopeWidget(bool bShowScope);

protected:
	virtual void BeginPlay() override;
	
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void EquipButtonPressed();
	
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

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

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

	void PlayHitReactMontage();

	UPROPERTY(EditDefaultsOnly)
	float CameraThreshold = 200.f;
	
	void HideCameraIfCharacterClose();

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
	void OnRep_Health();

	void UpdateHUDHealth();
#pragma endregion Health

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

	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	UMaterialInstance* DissolveMaterialInstance;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	UParticleSystem* EliminationEffect;

	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UParticleSystemComponent* EliminationComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Elimination")
	class USoundCue* EliminationSound;
#pragma endregion Elimination

	UPROPERTY()
	class ABasePlayerState* CharacterPlayerState;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

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

	ECombatState GetCombatState() const;

	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
};
