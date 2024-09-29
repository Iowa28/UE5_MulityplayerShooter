// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

protected:
	UPROPERTY(EditDefaultsOnly)
	float Damage = 20.f;
	
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditDefaultsOnly)
	class USoundCue* ImpactSound;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionBox;
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

private:
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* Tracer;

	UPROPERTY()
	UParticleSystemComponent* TracerComponent;
};
