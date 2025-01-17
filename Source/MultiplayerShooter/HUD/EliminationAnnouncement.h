// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EliminationAnnouncement.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API UEliminationAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* AnnouncementBox;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncementText;

	void SetAnnouncementText(FString AttackerName, FString VictimName);
};
