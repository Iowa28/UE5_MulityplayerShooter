// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseHUD.h"

void ABaseHUD::DrawHUD()
{
	Super::DrawHUD();

	if (GEngine)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

		DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter);
		DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter);
		DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter);
		DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter);
		DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter);
	}
}

void ABaseHUD::DrawCrosshair(UTexture2D* Crosshair, const FVector2D ViewportCenter)
{
	if (!Crosshair) { return; }
	
	const float CrosshairWidth = Crosshair->GetSizeX();
	const float CrosshairHeight = Crosshair->GetSizeY();
	const FVector2D CrosshairDrawPoint = FVector2D(
		ViewportCenter.X - CrosshairWidth / 2,
		ViewportCenter.Y - CrosshairHeight / 2
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
		FLinearColor::White
	);
}
