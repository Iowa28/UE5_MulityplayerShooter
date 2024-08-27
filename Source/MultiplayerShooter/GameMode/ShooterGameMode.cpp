// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameMode.h"

#include "MultiplayerShooter/Character/BaseCharacter.h"

void AShooterGameMode::PlayerEliminated(ABaseCharacter* EliminatedCharacter, ABasePlayerController* VictimController,
                                        ABasePlayerController* AttackerController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminate();
	}
}
