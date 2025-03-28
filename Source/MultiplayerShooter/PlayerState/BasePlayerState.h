// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MultiplayerShooter/Types/Team.h"
#include "BasePlayerState.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API ABasePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void AddToScore(float ScoreAmount);
	virtual void OnRep_Score() override;

	void AddToDefeats(int32 DefeatsAmount);

	UFUNCTION()
	virtual void OnRep_Defeats();

private:
	UPROPERTY()
	class ABaseCharacter* Character;
	
	UPROPERTY()
	class ABasePlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	UFUNCTION()
	void OnRep_Team();

public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(const ETeam TeamToSet);
};
