// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MultiplayerShooter/Character/BaseCharacter.h"
#include "MultiplayerShooter/HUD/BaseHUD.h"
#include "MultiplayerShooter/HUD/CharacterOverlay.h"

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();

	BaseHUD = Cast<ABaseHUD>(GetHUD());
}

void ABasePlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (const ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(aPawn))
	{
		SetHUDHealth(BaseCharacter->GetHealth(), BaseCharacter->GetMaxHealth());
	}
}

void ABasePlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->IsCharacterOverlayValid()) { return; }

	const float HealthPercent = Health / MaxHealth;
	BaseHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
	const FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
	BaseHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
}
