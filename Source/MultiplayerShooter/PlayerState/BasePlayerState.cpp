// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerState.h"

#include "MultiplayerShooter/Character/BaseCharacter.h"
#include "MultiplayerShooter/Controller/BasePlayerController.h"
#include "Net/UnrealNetwork.h"

void ABasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABasePlayerState, Defeats);
	DOREPLIFETIME(ABasePlayerState, Team);
}

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
	Super::OnRep_Score();//👍🤣😎

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

void ABasePlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;

	Character = Character ? Character : Cast<ABaseCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller ? Controller : Cast<ABasePlayerController>(Character->GetController());
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABasePlayerState::OnRep_Defeats()
{
	Character = Character ? Character : Cast<ABaseCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller ? Controller : Cast<ABasePlayerController>(Character->GetController());
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}
