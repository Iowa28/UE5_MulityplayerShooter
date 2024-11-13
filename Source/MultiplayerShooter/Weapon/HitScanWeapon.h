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
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditDefaultsOnly, Category = "HitScan")
	USoundCue* HitSound;
	
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

private:
	UPROPERTY(EditDefaultsOnly, Category = "HitScan")
	UParticleSystem* BeamParticles;

	UPROPERTY(EditDefaultsOnly, Category = "HitScan")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, Category = "HitScan")
	USoundCue* FireSound;

};
