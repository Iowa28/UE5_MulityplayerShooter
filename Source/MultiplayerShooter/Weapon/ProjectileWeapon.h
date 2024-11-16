// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

UCLASS(meta = (PrioritizeCategories ="Weapon"))
class MULTIPLAYERSHOOTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class AProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
	
};
