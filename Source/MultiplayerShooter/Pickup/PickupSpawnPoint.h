// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	UPROPERTY()
	APickup* SpawnedPickup;

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
	void SpawnPickupTimerFinished();
	void SpawnPickup();

private:
	UPROPERTY(EditDefaultsOnly)
	float SpawnPickupTimeMin;
	
	UPROPERTY(EditDefaultsOnly)
	float SpawnPickupTimeMax;

	FTimerHandle SpawnPickupTimer;
};
