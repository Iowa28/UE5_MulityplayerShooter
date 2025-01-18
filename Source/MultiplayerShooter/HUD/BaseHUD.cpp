// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseHUD.h"
#include "Announcement.h"
#include "CharacterOverlay.h"
#include "EliminationAnnouncement.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"

void ABaseHUD::BeginPlay()
{
	Super::BeginPlay();
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

void ABaseHUD::AddAnnouncement()
{
	if (AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(GetWorld(), AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ABaseHUD::AddEliminationAnnouncement(FString AttackerName, FString VictimName)
{
	OwningPlayer = OwningPlayer ? OwningPlayer : GetOwningPlayerController();
	
	if (OwningPlayer && EliminationAnnouncementClass)
	{
		if (UEliminationAnnouncement* EliminationAnnouncement = CreateWidget<UEliminationAnnouncement>(OwningPlayer, EliminationAnnouncementClass))
		{
			EliminationAnnouncement->SetAnnouncementText(AttackerName, VictimName);
			EliminationAnnouncement->AddToViewport();

			for (UEliminationAnnouncement* Message : EliminationMessages)
			{
				if (Message && Message->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Message->AnnouncementBox);
					if (CanvasSlot)
					{
						const FVector2D Position = CanvasSlot->GetPosition();
						const FVector2D NewPosition = FVector2D(Position.X, Position.Y - CanvasSlot->GetSize().Y);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}

			EliminationMessages.Add(EliminationAnnouncement);

			FTimerHandle EliminationMessageTimer;
			FTimerDelegate EliminationMessageDelegate;
			EliminationMessageDelegate.BindUFunction(this, FName("EliminationAnnouncementTimerFinished"), EliminationAnnouncement);
			GetWorldTimerManager().SetTimer(
				EliminationMessageTimer,
				EliminationMessageDelegate,
				EliminationAnnouncementTime,
				false
			);
		}
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

void ABaseHUD::EliminationAnnouncementTimerFinished(UEliminationAnnouncement* MessageToRemove)
{
	if (MessageToRemove)
	{
		MessageToRemove->RemoveFromParent();
	}
}
