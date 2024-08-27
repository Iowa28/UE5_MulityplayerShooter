// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BaseHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
	
	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

UCLASS()
class MULTIPLAYERSHOOTER_API ABaseHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool IsCharacterOverlayValid() const;

protected:
	virtual void BeginPlay() override;

	void AddCharacterOverlay();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	
	UPROPERTY(EditDefaultsOnly)
	float CrosshairSpreadMax = 16.f;
	
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Crosshair, FVector2D ViewportCenter, FVector2D Spread);

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
