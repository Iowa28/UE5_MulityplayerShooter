// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	// GEngine->AddOnScreenDebugMessage(-1,15.f,FColor::Yellow, FString::Printf(TEXT("Players number: %d"), NumberOfPlayers));
	if (NumberOfPlayers == 2)
	{
		if (UWorld* World = GetWorld())
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/Maps/ShooterMap?listen"));
		}
	}
}
