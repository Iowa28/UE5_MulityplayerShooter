// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/Character/BaseCharacter.h"
#include "MultiplayerShooter/Controller/BasePlayerController.h"
#include "MultiplayerShooter/GameState/ShooterGameState.h"
#include "MultiplayerShooter/PlayerState/BasePlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AShooterGameMode::AShooterGameMode()
{
	bDelayedStart = true;
}

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AShooterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void AShooterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABasePlayerController* PlayerController = Cast<ABasePlayerController>(*It))
		{
			PlayerController->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

float AShooterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

void AShooterGameMode::PlayerEliminated(ABaseCharacter* EliminatedCharacter, ABasePlayerController* VictimController,
                                        ABasePlayerController* AttackerController)
{
	ABasePlayerState* AttackerPlayerState = AttackerController ? AttackerController->GetPlayerState<ABasePlayerState>() : nullptr;
	ABasePlayerState* VictimPlayerState = VictimController ? VictimController->GetPlayerState<ABasePlayerState>() : nullptr;

	AShooterGameState* ShooterGameState = GetGameState<AShooterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && ShooterGameState)
	{
		TArray<ABasePlayerState*> PlayersCurrentlyInTheLead;
		for (ABasePlayerState* LeadPlayer : ShooterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}
		
		AttackerPlayerState->AddToScore(1.f);
		ShooterGameState->UpdateTopScore(AttackerPlayerState);
		if (ShooterGameState->TopScoringPlayers.Contains(AttackerPlayerState) && !PlayersCurrentlyInTheLead.Contains(AttackerPlayerState))
		{
			if (ABaseCharacter* Leader = Cast<ABaseCharacter>(AttackerPlayerState->GetPawn()))
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!ShooterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				if (ABaseCharacter* Loser = Cast<ABaseCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn()))
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminate(false);
	}

	if (AttackerPlayerState && VictimPlayerState)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ABasePlayerController* PlayerController = Cast<ABasePlayerController>(*It))
			{
				PlayerController->BroadcastElimination(AttackerPlayerState, VictimPlayerState);
			}
		}
	}
}

void AShooterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	
	if (EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		const int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[Selection]);
	}
}

void AShooterGameMode::PlayerLeftGame(ABasePlayerState* PlayerLeaving)
{
	if (!PlayerLeaving) { return; }

	AShooterGameState* ShooterGameState = GetGameState<AShooterGameState>();
	if (ShooterGameState && ShooterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		ShooterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}

	if (ABaseCharacter* CharacterLeaving = Cast<ABaseCharacter>(PlayerLeaving->GetPawn()))
	{
		CharacterLeaving->Eliminate(true);
	}
}
