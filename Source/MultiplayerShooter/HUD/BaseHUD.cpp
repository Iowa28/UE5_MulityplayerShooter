// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseHUD.h"
#include "CharacterOverlay.h"

void ABaseHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
}

void ABaseHUD::DrawHUD()
{
	Super::DrawHUD();

	if (GEngine)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);
		const float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, FVector2D());
		DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, FVector2D(-SpreadScaled, 0));
		DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, FVector2D(SpreadScaled, 0));
		DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, FVector2D(0, -SpreadScaled));
		DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, FVector2D(0, SpreadScaled));
	}
}

void ABaseHUD::AddCharacterOverlay()
{
	if (CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(GetWorld(), CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABaseHUD::DrawCrosshair(UTexture2D* Crosshair, const FVector2D ViewportCenter, const FVector2D Spread)
{
	if (!Crosshair) { return; }
	
	const float CrosshairWidth = Crosshair->GetSizeX();
	const float CrosshairHeight = Crosshair->GetSizeY();
	const FVector2D CrosshairDrawPoint = FVector2D(
		ViewportCenter.X - CrosshairWidth / 2 + Spread.X,
		ViewportCenter.Y - CrosshairHeight / 2 + Spread.Y
	);

	DrawTexture(
		Crosshair,
		CrosshairDrawPoint.X,
		CrosshairDrawPoint.Y,
		CrosshairWidth,
		CrosshairHeight,
		0,
		0,
		1,
		1,
		HUDPackage.CrosshairColor
	);
}
