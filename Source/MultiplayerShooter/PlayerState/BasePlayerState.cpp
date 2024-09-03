// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerState.h"

#include "MultiplayerShooter/Character/BaseCharacter.h"
#include "MultiplayerShooter/Controller/BasePlayerController.h"

void ABasePlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	Character = Character ? Character : Cast<ABaseCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller ? Controller : Cast<ABasePlayerController>(Character->GetController());
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ABasePlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character ? Character : Cast<ABaseCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller ? Controller : Cast<ABasePlayerController>(Character->GetController());
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}