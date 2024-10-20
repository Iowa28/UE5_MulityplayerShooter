// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "MultiplayerShooter/Character/BaseCharacter.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRamUp(DeltaTime);
	ShieldRamUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishingShield = true;
	ReplenishingRate = ShieldAmount / ReplenishTime;
	ReplenishShieldAmount += ShieldAmount;
}

void UBuffComponent::HealRamUp(float DeltaTime)
{
	if (!bHealing || !Character || Character->IsEliminated()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	const float HealthAmount = FMath::Min(Character->GetHealth() + HealThisFrame, Character->GetMaxHealth());
	Character->SetHealth(HealthAmount);
	Character->UpdateHUDHealth();
	AmountToHeal = FMath::Max(0.f, AmountToHeal - HealThisFrame);

	if (FMath::IsNearlyEqual(AmountToHeal, 0.f) || FMath::IsNearlyEqual(Character->GetHealth(), Character->GetMaxHealth()))
	{
		bHealing = false;
	}
}

void UBuffComponent::ShieldRamUp(float DeltaTime)
{
	if (!bReplenishingShield || !Character || Character->IsEliminated()) return;

	const float ReplenishThisFrame = ReplenishingRate * DeltaTime;
	const float ShieldAmount = FMath::Min(Character->GetShield() + ReplenishThisFrame, Character->GetMaxShield());
	Character->SetShield(ShieldAmount);
	Character->UpdateHUDShield();
	ReplenishShieldAmount = FMath::Max(0.f, ReplenishShieldAmount - ReplenishThisFrame);

	if (FMath::IsNearlyEqual(ReplenishShieldAmount, 0.f) || FMath::IsNearlyEqual(Character->GetShield(), Character->GetMaxShield()))
	{
		bReplenishingShield = false;
	}
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::BuffSpeed(float BaseSpeedBuff, float CrouchSpeedBuff, float BuffTime)
{
	if (!Character || !Character->GetCharacterMovement()) { return; }

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&ThisClass::ResetSpeed,
		BuffTime
	);

	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeedBuff;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeedBuff;
	MulticastBuffSpeed(BaseSpeedBuff, CrouchSpeedBuff);
}

void UBuffComponent::ResetSpeed()
{
	if (!Character || !Character->GetCharacterMovement()) { return; }

	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	MulticastBuffSpeed(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::MulticastBuffSpeed_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (!Character || !Character->GetCharacterMovement()) { return; }

	Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (!Character || !Character->GetCharacterMovement()) { return; }

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&ThisClass::ResetJump,
		BuffTime
	);

	Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	MulticastBuffJump(BuffJumpVelocity);
}

void UBuffComponent::ResetJump()
{
	if (!Character || !Character->GetCharacterMovement()) { return; }

	Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	MulticastBuffJump(InitialJumpVelocity);
}

void UBuffComponent::MulticastBuffJump_Implementation(float BuffJumpVelocity)
{
	if (!Character || !Character->GetCharacterMovement()) { return; }

	Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
}