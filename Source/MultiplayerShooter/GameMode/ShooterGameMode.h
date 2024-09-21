// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameMode.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AShooterGameMode();

	virtual void Tick(float DeltaSeconds) override;

	virtual void PlayerEliminated(class ABaseCharacter* EliminatedCharacter, class ABasePlayerController* VictimController,
	                              ABasePlayerController* AttackerController);

	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);

	UPROPERTY(EditDefaultsOnly, Category = "Warmup")
	float WarmupTime = 5.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
};
