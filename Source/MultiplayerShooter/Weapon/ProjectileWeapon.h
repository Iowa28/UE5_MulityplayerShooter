// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AProjectile> ProjectileClass;

public:
	virtual void Fire(const FVector& HitTarget) override;
};
