// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerController.h"
#include "Components/Image.h"
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
#include "MultiplayerShooter/HUD/ReturnToMainMenu.h"
#include "MultiplayerShooter/PlayerState/BasePlayerState.h"
#include "MultiplayerShooter/Types/Announcement.h"
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
	DOREPLIFETIME(ABasePlayerController, bShowTeamScores);
}

void ABasePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
	CheckPing(DeltaSeconds);
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
		if (bInitializeTeamScores)
		{
			InitTeamScores();
		}
		if (bInitializeCountdown)
		{
			SetHUDMatchCountdown(HUDCountdownTime);
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

void ABasePlayerController::ShowReturnToMainMenu()
{
	if (!ReturnToMainMenuWidget) { return; }

	ReturnToMainMenu = ReturnToMainMenu ? ReturnToMainMenu : CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

#pragma region EliminationAnnouncement
void ABasePlayerController::BroadcastElimination(APlayerState* Attacker, APlayerState* Victim)
{
	ClientEliminationAnnouncement(Attacker, Victim);
}

void ABasePlayerController::ClientEliminationAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	const APlayerState* Self = GetPlayerState<APlayerState>();
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (Attacker && Victim && Self && BaseHUD)
	{
		if (Attacker == Self && Victim != Self)
		{
			BaseHUD->AddEliminationAnnouncement("You", Victim->GetPlayerName());
		}
		else if (Victim == Self && Attacker != Self)
		{
			BaseHUD->AddEliminationAnnouncement(Attacker->GetPlayerName(), "you");
		}
		else if (Attacker == Victim && Attacker == Self)
		{
			BaseHUD->AddEliminationAnnouncement("You", "yourself kek");
		}
		else if (Attacker == Victim && Attacker != Self)
		{
			BaseHUD->AddEliminationAnnouncement(Attacker->GetPlayerName(), "themselves kek");
		}
		else
		{
			BaseHUD->AddEliminationAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}
#pragma endregion EliminationAnnouncement

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
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->MatchCountdownText)
	{
		HUDCountdownTime = CountdownTime;
		// bInitializeCountdown = true;
		return;
	}
	
	if (CountdownTime < 0.f)
	{
		BaseHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
		return;
	}

	bInitializeCountdown = false;
	const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
	const int32 Seconds = CountdownTime - Minutes * 60;
	
	const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	BaseHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
}

void ABasePlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->Announcement || !BaseHUD->Announcement->WarmupTime)
	{
		return;
	}

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
	SingleTripTime = RoundTripTime / 2;
	const float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
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

void ABasePlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
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

void ABasePlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority())
	{
		bShowTeamScores = bTeamsMatch;
	}
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (BaseHUD)
	{
		if (!BaseHUD->CharacterOverlay)
		{
			BaseHUD->AddCharacterOverlay();
		}
		if (!BaseHUD->Announcement)
		{
			BaseHUD->AddAnnouncement();
		}
		if (BaseHUD->Announcement)
		{
			BaseHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (!HasAuthority()) { return; }
	if (bTeamsMatch)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

void ABasePlayerController::HandleCooldown()
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (BaseHUD)
	{
		BaseHUD->CharacterOverlay->RemoveFromParent();
		if (!BaseHUD->Announcement)
		{
			BaseHUD->AddAnnouncement();
		}
		if (BaseHUD->Announcement && BaseHUD->Announcement->AnnouncementText && BaseHUD->Announcement->InfoText)
		{
			BaseHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			BaseHUD->Announcement->AnnouncementText->SetText(FText::FromString(Announcement::NewMatchStartsIn));
			
			if (const AShooterGameState* GameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this)))
			{
				const TArray<ABasePlayerState*> TopPlayers = GameState->TopScoringPlayers;
				const FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(GameState) : GetInfoText(TopPlayers);
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

FString ABasePlayerController::GetInfoText(const TArray<ABasePlayerState*>& Players) const
{
	FString InfoTextString;
	
	const ABasePlayerState* ThisPlayerState = GetPlayerState<ABasePlayerState>();
	if (!ThisPlayerState) { return InfoTextString; }
	
	if (Players.Num() <= 0)
	{
		InfoTextString = Announcement::NobodyWon;
	}
	else if (Players.Num() == 1 && Players[0] == ThisPlayerState)
	{
		InfoTextString = Announcement::YouWon;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("%s won."), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::Winners;
		InfoTextString.Append(FString(""));
		for (const ABasePlayerState* TopPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TopPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

FString ABasePlayerController::GetTeamsInfoText(const AShooterGameState* ShooterGameState) const
{
	FString TeamsInfoText;
	const int32 RedTeamScore = ShooterGameState->RedTeamScore;
	const int32 BlueTeamScore = ShooterGameState->BlueTeamScore;
	if (RedTeamScore > BlueTeamScore)
	{
		TeamsInfoText = Announcement::RedTeamWon;
	}
	else if (RedTeamScore < BlueTeamScore)
	{
		TeamsInfoText = Announcement::BlueTeamWon;
	}
	else
	{
		TeamsInfoText = Announcement::NobodyWon;
	}

	return TeamsInfoText;
}

#pragma endregion Match

#pragma region Ping

void ABasePlayerController::CheckPing(float DeltaSeconds)
{
	HighPingRunningTime += DeltaSeconds;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		// PlayerState = PlayerState ? PlayerState : GetPlayerState<APlayerState>();
		if (PlayerState)
		{
			const float Ping = PlayerState->GetCompressedPing() * 4;
			if (GetPawn() && GetPawn()->IsLocallyControlled())
			{
			}
			if (Ping > HighPingThreshold)
			{
				StartHighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}

	if (BaseHUD && BaseHUD->CharacterOverlay && BaseHUD->CharacterOverlay->HighPingAnimation && BaseHUD->CharacterOverlay->IsAnimationPlaying(BaseHUD->CharacterOverlay->HighPingAnimation))
	{
		PingAnimationRunningTime += DeltaSeconds;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABasePlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void ABasePlayerController::StartHighPingWarning()
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->HighPingImage || !BaseHUD->CharacterOverlay->HighPingAnimation)
	{
		return;
	}
	
	BaseHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
	BaseHUD->CharacterOverlay->PlayAnimation(BaseHUD->CharacterOverlay->HighPingAnimation, 0.f, 5);
}

void ABasePlayerController::StopHighPingWarning()
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->HighPingImage || !BaseHUD->CharacterOverlay->HighPingAnimation)
	{
		return;
	}
	
	BaseHUD->CharacterOverlay->HighPingImage->SetOpacity(0);
	if (BaseHUD->CharacterOverlay->IsAnimationPlaying(BaseHUD->CharacterOverlay->HighPingAnimation))
	{
		BaseHUD->CharacterOverlay->StopAnimation(BaseHUD->CharacterOverlay->HighPingAnimation);
	}
}
#pragma endregion Ping

#pragma region TeamScore

void ABasePlayerController::HideTeamScores()
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->RedTeamScore || !BaseHUD->CharacterOverlay->BlueTeamScore || !BaseHUD->CharacterOverlay->ScoreSpacer)
	{
		bInitializeTeamScores = true;
		return;
	}

	bInitializeTeamScores = false;
	BaseHUD->CharacterOverlay->RedTeamScore->SetText(FText());
	BaseHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
	BaseHUD->CharacterOverlay->ScoreSpacer->SetText(FText());
}

void ABasePlayerController::InitTeamScores()
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->RedTeamScore || !BaseHUD->CharacterOverlay->BlueTeamScore || !BaseHUD->CharacterOverlay->ScoreSpacer)
	{
		bInitializeTeamScores = true;
		return;
	}

	bInitializeTeamScores = false;
	BaseHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(FString("0")));
	BaseHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(FString("0")));
	BaseHUD->CharacterOverlay->ScoreSpacer->SetText(FText::FromString(FString("|")));
}

void ABasePlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->RedTeamScore)
	{
		return;
	}

	const FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
	BaseHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
}

void ABasePlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	BaseHUD = BaseHUD ? BaseHUD : Cast<ABaseHUD>(GetHUD());
	if (!BaseHUD || !BaseHUD->CharacterOverlay || !BaseHUD->CharacterOverlay->BlueTeamScore)
	{
		return;
	}

	const FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
	BaseHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
}

void ABasePlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

#pragma endregion TeamScore
