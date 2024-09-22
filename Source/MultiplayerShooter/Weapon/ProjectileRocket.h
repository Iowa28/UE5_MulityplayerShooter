// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float MinimumDamage = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float DamageInnerRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float DamageOuterRadius = 500.f;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	UStaticMeshComponent* RocketMesh;
};
