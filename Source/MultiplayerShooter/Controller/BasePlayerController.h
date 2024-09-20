// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;
	
	void SetHUDHealth(float Health, float MaxHealth);

	void SetHUDScore(float Score);

	void SetHUDDefeats(int32 Defeats);

	void SetHUDWeaponAmmo(int32 Ammo);

	void SetHUDCarriedAmmo(int32 Ammo);

	void SetHUDMatchCountdown(float CountdownTime);

	float GetServerTime() const;

	virtual void ReceivedPlayer() override;

protected:
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* aPawn) override;

	void SetHUDTime();

	UPROPERTY(EditDefaultsOnly, Category = "Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	
	float ClientServerDelta = 0.f;

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

private:
	UPROPERTY()
	class ABaseHUD* BaseHUD;

	float MatchTime = 120.f;
	uint32 CountdownInt = 0;

	void CheckTimeSync(float DeltaSeconds);
};
