// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

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
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::HealRamUp(float DeltaTime)
{
	if (!bHealing || !Character || Character->IsEliminated()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	const float HealthAmount = FMath::Min(Character->GetHealth() + HealThisFrame, Character->GetMaxHealth());
	Character->SetHealth(HealthAmount);
	// UE_LOG(LogTemp, Warning, TEXT("%f"), HealthAmount);
	Character->UpdateHUDHealth();
	AmountToHeal = FMath::Max(0.f, AmountToHeal - HealThisFrame);

	if (FMath::IsNearlyEqual(AmountToHeal, 0.f) || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
	}
}
