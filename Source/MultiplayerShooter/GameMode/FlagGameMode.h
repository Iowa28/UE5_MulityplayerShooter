// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "FlagGameMode.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(ABaseCharacter* EliminatedCharacter, ABasePlayerController* VictimController,
		ABasePlayerController* AttackerController) override;

	void CaptureTheFlag(class AFlag* Flag, class AFlagZone* Zone);
};
