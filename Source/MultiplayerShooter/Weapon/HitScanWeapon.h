// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

UCLASS(meta = (PrioritizeCategories ="HitScan Scatter Weapon"))
class MULTIPLAYERSHOOTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "HitScan")
	float Damage = 20.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "HitScan")
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditDefaultsOnly, Category = "HitScan")
	USoundCue* HitSound;
	
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

private:
	UPROPERTY(EditDefaultsOnly, Category = "HitScan")
	UParticleSystem* BeamParticles;

	UPROPERTY(EditDefaultsOnly, Category = "HitScan")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, Category = "HitScan")
	USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditDefaultsOnly, Category = "Scatter")
	bool bUseScatter = false;
};
