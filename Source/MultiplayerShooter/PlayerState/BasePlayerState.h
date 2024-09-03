// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BasePlayerState.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API ABasePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void OnRep_Score() override;

	void AddToScore(float ScoreAmount);

private:
	class ABaseCharacter* Character;
	class ABasePlayerController* Controller;
};
