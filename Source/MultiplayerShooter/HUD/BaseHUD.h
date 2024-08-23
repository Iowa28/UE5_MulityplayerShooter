// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BaseHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
	
	// FHUDPackage(UTexture2D* Center, UTexture2D* Left, UTexture2D* Right, UTexture2D* Top, UTexture2D* Bottom)
	// : CrosshairCenter(Center), CrosshairLeft(Left), CrosshairRight(Right), CrosshairTop(Top), CrosshairBottom(Bottom) {}
	
	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
};

UCLASS()
class MULTIPLAYERSHOOTER_API ABaseHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	FHUDPackage HUDPackage;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
