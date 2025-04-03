// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) { return; }

	const UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	check(Subsystem);

	const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("NumberOfPlayers: %d"), NumberOfPlayers));
	//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("DesiredNumPublicConnections: %d"), Subsystem->DesiredNumPublicConnections));
	
	if (NumberOfPlayers == Subsystem->DesiredNumPublicConnections)
	{
		if (UWorld* World = GetWorld())
		{
			bUseSeamlessTravel = true;
			const FString MatchType = Subsystem->DesiredMatchType;
			if (MatchType == "FreeForAll")
			{
				World->ServerTravel(FString("/Game/Maps/ShooterMap?listen"));
			}
			else if (MatchType == "Teams")
			{
				World->ServerTravel(FString("/Game/Maps/TeamMap?listen"));
			}
			else if (MatchType == "Flags")
			{
				World->ServerTravel(FString("/Game/Maps/FlagMap?listen"));
			}
		}
	}
}
