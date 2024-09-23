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
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	class UNiagaraSystem* TrailSystem;
	
	virtual void BeginPlay() override;
	
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                   FVector NormalImpulse, const FHitResult& Hit) override;
	
	void DestroyTimerFinished();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float MinimumDamage = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float DamageInnerRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float DamageOuterRadius = 500.f;
	
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	UStaticMeshComponent* RocketMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float DestroyTime = 3.f;
	FTimerHandle DestroyTimer;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	USoundCue* ProjectileLoop;
	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	USoundAttenuation* LoopingSoundAttenuation;
	
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;
	
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;
};
