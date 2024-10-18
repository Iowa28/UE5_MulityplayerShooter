// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "MultiplayerShooter/Types/WeaponTypes.h"
#include "AmmoPickup.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AAmmoPickup : public APickup
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	int32 AmmoAmount = 30;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	EWeaponType WeaponType;
};
