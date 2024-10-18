// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "MultiplayerShooter/Components/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (UCombatComponent* CombatComponent = OtherActor->GetComponentByClass<UCombatComponent>())
	{
		CombatComponent->PickupAmmo(WeaponType, AmmoAmount);
	}

	Destroy();
}
