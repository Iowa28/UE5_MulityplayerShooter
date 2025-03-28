// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERSHOOTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	
	friend class ABaseCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Heal(float HealAmount, float HealingTime);
	void ReplenishShield(float ShieldAmount, float ReplenishTime);

	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);
	void BuffSpeed(float BaseSpeedBuff, float CrouchSpeedBuff, float BuffTime);

	void SetInitialJumpVelocity(float Velocity);
	void BuffJump(float BuffJumpVelocity, float BuffTime);

protected:
	virtual void BeginPlay() override;

	void HealRamUp(float DeltaTime);
	void ShieldRamUp(float DeltaTime);

private:
	UPROPERTY()
	ABaseCharacter* Character;

	bool bHealing = false;
	float HealingRate = 0;
	float AmountToHeal;

	bool bReplenishingShield = false;
	float ReplenishingRate = 0;
	float ReplenishShieldAmount;

	FTimerHandle SpeedBuffTimer;
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBuffSpeed(float BaseSpeed, float CrouchSpeed);
	void ResetSpeed();

	FTimerHandle JumpBuffTimer;
	float InitialJumpVelocity;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBuffJump(float BuffJumpVelocity);
	void ResetJump();	
};
