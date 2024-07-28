// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiplayerShooterGameMode.h"
#include "MultiplayerShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMultiplayerShooterGameMode::AMultiplayerShooterGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Character/BP_Character"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
