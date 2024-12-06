// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bHighPing);

UCLASS()
class MULTIPLAYERSHOOTER_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);

	float GetServerTime() const;

	virtual void ReceivedPlayer() override;

	void OnMatchStateSet(FName State);

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

protected:
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* aPawn) override;

	void SetHUDTime();

	void PollInit();

	UPROPERTY(EditDefaultsOnly, Category = "Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	
	float ClientServerDelta = 0.f;

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	void HandleMatchHasStarted();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown);

	void CheckPing(float DeltaSeconds);
	void  StartHighPingWarning();
	void  StopHighPingWarning();

private:
	UPROPERTY()
	class ABaseHUD* BaseHUD;

	UPROPERTY()
	class AShooterGameMode* GameMode;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	void CheckTimeSync(float DeltaSeconds);

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	float HUDHealth, HUDMaxHealth;
	bool bInitializeHealth = false;
	float HUDShield, HUDMaxShield;
	bool bInitializeShield = false;
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats = false;
	int32 HUDGrenades;
	bool bInitializeGrenades = false;
	int32 HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
	int32 HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

	void HandleCooldown();

	float HighPingRunningTime = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ping")
	float HighPingDuration = 5.f;

	float PingAnimationRunningTime = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ping")
	float CheckPingFrequency = 20.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Ping")
	float HighPingThreshold = 50.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);
};
