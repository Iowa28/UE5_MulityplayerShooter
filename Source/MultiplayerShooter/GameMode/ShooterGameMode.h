// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameMode.generated.h"

namespace MatchState
{
	extern MULTIPLAYERSHOOTER_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

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

	UPROPERTY(EditDefaultsOnly, Category = "Match Settings")
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Match Settings")
	float WarmupTime = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Match Settings")
	float CooldownTime = 5.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
