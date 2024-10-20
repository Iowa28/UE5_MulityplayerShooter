// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPickup.h"
#include "MultiplayerShooter/Components/BuffComponent.h"

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (UBuffComponent* BuffComponent = OtherActor->GetComponentByClass<UBuffComponent>())
	{
		BuffComponent->BuffJump(JumpZVelocityBuff, JumpBuffTime);
	}

	Destroy();
}
