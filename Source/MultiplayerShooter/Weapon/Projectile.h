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
	
	float Damage = 20.f;

	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditDefaultsOnly)
	float InitialSpeed = 15000.f;

protected:
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditDefaultsOnly)
	class USoundCue* ImpactSound;
		
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionBox;
	
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;
	
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SpawnTrailSystem();
	void ExplodeDamage();

	void StartDestroyTimer();
	void DestroyTimerFinished();

private:
	UPROPERTY(EditDefaultsOnly)
	float MinimumDamage = 10.f;
	
	UPROPERTY(EditDefaultsOnly)
	float DamageInnerRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly)
	float DamageOuterRadius = 500.f;
	
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* Tracer;

	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditDefaultsOnly)
	float DestroyTime = 3.f;
	
	FTimerHandle DestroyTimer;
};
