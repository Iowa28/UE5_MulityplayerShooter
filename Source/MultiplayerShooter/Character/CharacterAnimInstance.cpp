// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerShooter/Weapon/Weapon.h"

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ABaseCharacter>(TryGetPawnOwner());
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Character)
	{
		Character = Cast<ABaseCharacter>(TryGetPawnOwner());
	}

	if (!Character) { return; }

	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = Character->GetCharacterMovement()->IsFalling();
	bIsAccelerating = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bWeaponEquipped = Character->IsWeaponEquipped();
	EquippedWeapon = Character->GetEquippedWeapon();
	bIsCrouched = Character->bIsCrouched;
	bAiming = Character->IsAiming();
	TurningInPlace = Character->GetTurningInPlace();
	bRotateRootBone = Character->ShouldRotateRootBone();
	bEliminated = Character->IsEliminated();

	const FRotator AimRotation = Character->GetBaseAimRotation();
	const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Character->GetVelocity());
	const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = Character->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AimOffsetYaw = Character->GetAimOffsetYaw();
	AimOffsetPitch = Character->GetAimOffsetPitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && Character->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"));
		FVector OutPosition;
		FRotator OutRotation;
		Character->GetMesh()->TransformToBoneSpace(
			FName("Hand_R"),
			LeftHandTransform.GetLocation(),
			FRotator::ZeroRotator,
			OutPosition,
			OutRotation
		);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (Character->IsLocallyControlled())
		{
			bLocallyControlled = true;
			const FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"));
			const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
				RightHandTransform.GetLocation(),
				RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - Character->GetHitTarget())
			);
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 30.f);
		}
	}

	bUseFABRIK = Character->GetCombatState() == ECombatState::ECS_Unoccupied;
	bUseAimOffsets = Character->GetCombatState() == ECombatState::ECS_Unoccupied && !Character->bDisableGameplay;
	bTransformRightHand = Character->GetCombatState() == ECombatState::ECS_Unoccupied && !Character->bDisableGameplay;
}
