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
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);

protected:
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* aPawn) override;

private:
	UPROPERTY()
	class ABaseHUD* BaseHUD;
};
