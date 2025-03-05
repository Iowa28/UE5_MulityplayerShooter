// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/Controller/BasePlayerController.h"
#include "MultiplayerShooter/GameState/ShooterGameState.h"
#include "MultiplayerShooter/PlayerState/BasePlayerState.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this)))
	{
		ABasePlayerState* BasePlayerState = NewPlayer->GetPlayerState<ABasePlayerState>();
		if (BasePlayerState && BasePlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (ShooterGameState->BlueTeam.Num() >= ShooterGameState->RedTeam.Num())
			{
				ShooterGameState->RedTeam.AddUnique(BasePlayerState);
				BasePlayerState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				ShooterGameState->BlueTeam.AddUnique(BasePlayerState);
				BasePlayerState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	ABasePlayerState* BasePlayerState = Exiting->GetPlayerState<ABasePlayerState>();
	if (ShooterGameState && BasePlayerState)
	{
		if (ShooterGameState->RedTeam.Contains(BasePlayerState))
		{
			ShooterGameState->RedTeam.Remove(BasePlayerState);
		}
		else if (ShooterGameState->BlueTeam.Contains(BasePlayerState))
		{
			ShooterGameState->BlueTeam.Remove(BasePlayerState);
		}
	}

	Super::Logout(Exiting);
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	const ABasePlayerState* AttackerPlayerState = Attacker->GetPlayerState<ABasePlayerState>();
	const ABasePlayerState* VictimPlayerState = Victim->GetPlayerState<ABasePlayerState>();
	if (!AttackerPlayerState || !VictimPlayerState || AttackerPlayerState == VictimPlayerState) { return BaseDamage; }
	if (AttackerPlayerState->GetTeam() == VictimPlayerState->GetTeam())
	{
		return 0;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(ABaseCharacter* EliminatedCharacter, ABasePlayerController* VictimController,
	ABasePlayerController* AttackerController)
{
	Super::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);

	AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	if (!ShooterGameState || !AttackerController) { return; }

	const ABasePlayerState* AttackerPlayerState = Cast<ABasePlayerState>(AttackerController->PlayerState);
	if (!AttackerPlayerState) { return; }

	if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
	{
		ShooterGameState->BlueTeamScores();
	}
	else if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
	{
		ShooterGameState->RedTeamScores();
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this)))
	{
		for (TObjectPtr<APlayerState> PlayerState : ShooterGameState->PlayerArray)
		{
			ABasePlayerState* BasePlayerState = Cast<ABasePlayerState>(PlayerState.Get());
			if (BasePlayerState && BasePlayerState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (ShooterGameState->BlueTeam.Num() >= ShooterGameState->RedTeam.Num())
				{
					ShooterGameState->RedTeam.AddUnique(BasePlayerState);
					BasePlayerState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					ShooterGameState->BlueTeam.AddUnique(BasePlayerState);
					BasePlayerState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}
