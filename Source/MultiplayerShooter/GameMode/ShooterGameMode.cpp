// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/Character/BaseCharacter.h"
#include "MultiplayerShooter/Controller/BasePlayerController.h"
#include "MultiplayerShooter/PlayerState/BasePlayerState.h"

void AShooterGameMode::PlayerEliminated(ABaseCharacter* EliminatedCharacter, ABasePlayerController* VictimController,
                                        ABasePlayerController* AttackerController)
{
	ABasePlayerState* AttackerPlayerState = AttackerController ? AttackerController->GetPlayerState<ABasePlayerState>() : nullptr;
	const ABasePlayerState* VictimPlayerState = VictimController ? VictimController->GetPlayerState<ABasePlayerState>() : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminate();
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
