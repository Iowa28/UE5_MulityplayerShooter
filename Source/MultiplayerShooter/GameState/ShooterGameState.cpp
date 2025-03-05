// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameState.h"
#include "MultiplayerShooter/Controller/BasePlayerController.h"
#include "MultiplayerShooter/PlayerState/BasePlayerState.h"
#include "Net/UnrealNetwork.h"

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterGameState, TopScoringPlayers);
	DOREPLIFETIME(AShooterGameState, RedTeamScore);
	DOREPLIFETIME(AShooterGameState, BlueTeamScore);
}

void AShooterGameState::UpdateTopScore(ABasePlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (FMath::IsNearlyEqual(ScoringPlayer->GetScore(), TopScore))
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AShooterGameState::RedTeamScores()
{
	RedTeamScore++;
	
	Controller = Controller ? Controller : Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());
	if (Controller)
	{
		Controller->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AShooterGameState::OnRep_RedTeamScore()
{
	Controller = Controller ? Controller : Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());
	if (Controller)
	{
		Controller->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AShooterGameState::BlueTeamScores()
{
	BlueTeamScore++;

	Controller = Controller ? Controller : Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());
	if (Controller)
	{
		Controller->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void AShooterGameState::OnRep_BlueTeamScore()
{
	Controller = Controller ? Controller : Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());
	if (Controller)
	{
		Controller->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
