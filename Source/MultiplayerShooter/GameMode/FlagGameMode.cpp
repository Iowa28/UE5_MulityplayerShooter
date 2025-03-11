// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagGameMode.h"
#include "MultiplayerShooter/Character/FlagZone.h"
#include "MultiplayerShooter/GameState/ShooterGameState.h"
#include "MultiplayerShooter/Weapon/Flag.h"

void AFlagGameMode::PlayerEliminated(ABaseCharacter* EliminatedCharacter, ABasePlayerController* VictimController,
                                     ABasePlayerController* AttackerController)
{
	AShooterGameMode::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
}

void AFlagGameMode::CaptureTheFlag(AFlag* Flag, AFlagZone* Zone)
{
	if (!Flag || !Zone || Flag->GetTeam() == Zone->Team) { return; }
	
	if (AShooterGameState* ShooterGameState = Cast<AShooterGameState>(GameState))
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			ShooterGameState->BlueTeamScores();
		}
		else if (Zone->Team == ETeam::ET_RedTeam)
		{
			ShooterGameState->RedTeamScores();
		}
	}
}
