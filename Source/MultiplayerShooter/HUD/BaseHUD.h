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

	UPROPERTY()
	class UAnnouncement* Announcement;

	// UPROPERTY()
	// class UEliminationAnnouncement* EliminationAnnouncement;

	void AddCharacterOverlay();
	void AddAnnouncement();
	void AddEliminationAnnouncement(FString AttackerName, FString VictimName);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UPROPERTY(EditDefaultsOnly, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY(EditDefaultsOnly, Category = "Announcements")
	TSubclassOf<class UEliminationAnnouncement> EliminationAnnouncementClass;
	
	UPROPERTY(EditDefaultsOnly)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY()
	APlayerController* OwningPlayer;
	
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Crosshair, FVector2D ViewportCenter, FVector2D Spread);

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
