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
	
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                   FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(VisibleDefaultsOnly)
	class URocketMovementComponent* RocketMovementComponent;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	USoundCue* ProjectileLoop;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	USoundAttenuation* LoopingSoundAttenuation;
	
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;
};
