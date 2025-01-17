// Fill out your copyright notice in the Description page of Project Settings.


#include "EliminationAnnouncement.h"

#include "Components/TextBlock.h"

void UEliminationAnnouncement::SetAnnouncementText(FString AttackerName, FString VictimName)
{
	const FString EliminationText = FString::Printf(TEXT("%s eliminated %s!"), *AttackerName, *VictimName);
	if (AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(EliminationText));
	}
}
