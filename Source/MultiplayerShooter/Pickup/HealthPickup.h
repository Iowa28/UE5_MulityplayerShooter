// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:
	AHealthPickup();

	virtual void Destroyed() override;

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float HealAmount = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	float HealingTime = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	class UNiagaraComponent* PickupEffectComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	class UNiagaraSystem* PickupEffect;
};