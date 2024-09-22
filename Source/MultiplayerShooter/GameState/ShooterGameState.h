// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShooterGameState.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AShooterGameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated)
	TArray<class ABasePlayerState*> TopScoringPlayers;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(ABasePlayerState* ScoringPlayer);

private:
	float TopScore = 0.f;
};
