// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/Character/BaseCharacter.h"
#include "MultiplayerShooter/Components/CombatComponent.h"
#include "MultiplayerShooter/GameMode/ShooterGameMode.h"
#include "MultiplayerShooter/GameState/ShooterGameState.h"
#include "MultiplayerShooter/HUD/Announcement.h"
#include "MultiplayerShooter/HUD/BaseHUD.h"
#include "MultiplayerShooter/HUD/CharacterOverlay.h"
#include "MultiplayerShooter/PlayerState/BasePlayerState.h"
#include "Net/UnrealNetwork.h"

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();

	BaseHUD = Cast<ABaseHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABasePlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (const ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(aPawn))
	{
		SetHUDHealth(BaseCharacter->GetHealth(), BaseCharacter->GetMaxHealth());
		SetHUDShield(BaseCharacter->GetShield(), BaseCharacter->GetMaxShield());
	}
}

void ABasePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABasePlayerController, MatchState);
}

void ABasePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckTimeSync(DeltaSeconds);
	SetHUDTime();
	PollInit();
}

void ABasePlayerController::PollInit()
{
	if (!CharacterOverlay && BaseHUD && BaseHUD->CharacterOverlay)
	{
		CharacterOverlay = BaseHUD->CharacterOverlay;
		if (bInitializeHealth)
		{
			SetHUDHealth(HUDHealth, HUDMaxHealth);
		}
		if (bInitializeShield)
		{
			SetHUDShield(HUDShield, HUDMaxShield);
		}
		if (bInitializeScore)
		{
			SetHUDScore(HUDScore);
		}
		if (bInitializeDefeats)
		{
			SetHUDDefeats(HUDDefeats);
		}
		if (bInitializeWeaponAmmo)
		{
			SetHUDWeaponAmmo(HUDWeaponAmmo);
		}
		if (bInitializeCarriedAmmo)
		{
			SetHUDCarriedAmmo(HUDCarriedAmmo);
		}

		if (bInitializeGrenades && GetPawn())
		{
			if (const UCombatComponent* CombatComponent = GetPawn()->GetComponentByClass<UCombatComponent>())
			{
				SetHUDGrenades(CombatComponent->GetGrenades());
			}
		}
	}
}

#pragma region HUD
void ABasePlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->HealthBar || !BaseHUD->CharacterOverlay->HealthText)
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
		return;
	}
	
	bInitializeHealth = false;
	const float HealthPercent = Health / MaxHealth;
	BaseHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
	const FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::RoundToInt(Health), FMath::RoundToInt(MaxHealth));
	BaseHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
}

void ABasePlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->ShieldBar || !BaseHUD->CharacterOverlay->ShieldText)
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
		return;
	}

	bInitializeShield = false;
	const float ShieldPercent = Shield / MaxShield;
	BaseHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
	const FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::RoundToInt(Shield), FMath::RoundToInt(MaxShield));
	BaseHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
}

void ABasePlayerController::SetHUDScore(float Score)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->ScoreAmount)
	{
		bInitializeScore = true;
		HUDScore = Score;
		return;
	}

	bInitializeScore = false;
	const FString ScoreText = FString::FromInt(FMath::FloorToInt(Score));
	BaseHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
}

void ABasePlayerController::SetHUDDefeats(int32 Defeats)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->DefeatsAmount)
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
		return;
	}

	bInitializeDefeats = false;
	const FString DefeatsText = FString::FromInt(Defeats);
	BaseHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
}

void ABasePlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
		return;
	}

	bInitializeWeaponAmmo = false;
	const FString AmmoText = FString::FromInt(Ammo);
	BaseHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
}

void ABasePlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
		return;
	}

	bInitializeCarriedAmmo = false;
	const FString AmmoText = FString::FromInt(Ammo);
	BaseHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
}

void ABasePlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->MatchCountdownText) { return; }

	if (CountdownTime < 0.f)
	{
		BaseHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
		return;
	}

	const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
	const int32 Seconds = CountdownTime - Minutes * 60;
	
	const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	BaseHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
}

void ABasePlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->Announcement || !BaseHUD->Announcement->WarmupTime) { return; }

	if (CountdownTime < 0.f)
	{
		BaseHUD->Announcement->WarmupTime->SetText(FText());
		return;
	}

	const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
	const int32 Seconds = CountdownTime - Minutes * 60;
	
	const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	BaseHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
}

void ABasePlayerController::SetHUDGrenades(int32 Grenades)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->GrenadesText)
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
		return;
	}

	bInitializeGrenades = false;
	const FString GrenadesText = FString::FromInt(Grenades);
	BaseHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
}

void ABasePlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		GameMode = GameMode ? GameMode : Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
		if (GameMode)
		{
			SecondsLeft = FMath::CeilToInt(GameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
		
	if (SecondsLeft != CountdownInt)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}
#pragma endregion HUD

#pragma region TimeCalculation
void ABasePlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0;
	}
}

float ABasePlayerController::GetServerTime() const
{
	return HasAuthority() ? GetWorld()->GetTimeSeconds() : GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABasePlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABasePlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABasePlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	const float CurrentServerTime = TimeServerReceivedClientRequest + RoundTripTime / 2;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

#pragma endregion TimeCalculation

#pragma region Match
void ABasePlayerController::ServerCheckMatchState_Implementation()
{
	GameMode = GameMode ? GameMode : Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);
	}
}

void ABasePlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown)
{
	MatchState = StateOfMatch;
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	OnMatchStateSet(MatchState);

	if (BaseHUD && MatchState == MatchState::WaitingToStart)
	{
		BaseHUD->AddAnnouncement();
	}
}

void ABasePlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABasePlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABasePlayerController::HandleMatchHasStarted()
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (BaseHUD)
	{
		if (!BaseHUD->CharacterOverlay)
		{
			BaseHUD->AddCharacterOverlay();
		}
		if (BaseHUD->Announcement)
		{
			BaseHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABasePlayerController::HandleCooldown()
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (BaseHUD)
	{
		BaseHUD->CharacterOverlay->RemoveFromParent();
		if (BaseHUD->Announcement && BaseHUD->Announcement->AnnouncementText && BaseHUD->Announcement->InfoText)
		{
			BaseHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			const FString AnnouncementText = FString("New match starts in:");
			BaseHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			const AShooterGameState* GameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
			const ABasePlayerState* ThisPlayerState = GetPlayerState<ABasePlayerState>();
			if (GameState && ThisPlayerState)
			{
				TArray<ABasePlayerState*> TopPlayers = GameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() <= 0)
				{
					InfoTextString = FString("Nobody won.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == ThisPlayerState)
				{
					InfoTextString = FString("You won!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("%s won."), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Winners:\n");
					for (const ABasePlayerState* TopPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TopPlayer->GetPlayerName()));
					}
				}
				
				BaseHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}

	if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(GetPawn()))
	{
		BaseCharacter->bDisableGameplay = true;
		if (BaseCharacter->GetCombatComponent())
		{
			BaseCharacter->GetCombatComponent()->FireButtonPressed(false);
		}
	}
}
#pragma endregion Match
