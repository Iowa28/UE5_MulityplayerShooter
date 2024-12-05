// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerShooter/Character/BaseCharacter.h"
#include "MultiplayerShooter/Components/LagCompensationComponent.h"
#include "MultiplayerShooter/Controller/BasePlayerController.h"
#include "Sound/SoundCue.h"

void AShotgun::ShotgunFire(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) { return; }
	AController* InstigatorController = OwnerPawn->GetController();
	if (!InstigatorController) { return; }

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleFlashSocket) { return; }

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransform.GetLocation();

	// Maps hit character to number of times hit
	TMap<ABaseCharacter*, uint32> HitMap;
	for (FVector_NetQuantize HitTarget : HitTargets)
	{
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		ABaseCharacter* Character = Cast<ABaseCharacter>(FireHit.GetActor());
		if (Character)
		{
			if (HitMap.Contains(Character))
			{
				HitMap[Character]++;
			}
			else
			{
				HitMap.Emplace(Character, 1);
			}

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					.5f,
					FMath::FRandRange(-.5f, .5f)
				);
			}
		}
	}

	TArray<ABaseCharacter*> HitCharacters;
	for (TTuple<ABaseCharacter*, uint32> HitPair : HitMap)
	{
		if (!HitPair.Key || !InstigatorController) { return; }
		if (HasAuthority() && OwnerPawn->IsLocallyControlled())
		{
			UGameplayStatics::ApplyDamage(
				HitPair.Key,
				Damage * HitPair.Value,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
		}
		HitCharacters.Add(HitPair.Key);
	}

	if (!HasAuthority() && bUseServerSideRewind && OwnerPawn->IsLocallyControlled())
	{
		OwnerCharacter = OwnerCharacter ? OwnerCharacter : Cast<ABaseCharacter>(OwnerPawn);
		OwnerController = OwnerController ? OwnerController : Cast<ABasePlayerController>(InstigatorController);
		if (OwnerCharacter && OwnerController && OwnerCharacter->GetLagCompensationComponent())
		{
			OwnerCharacter->GetLagCompensationComponent()->ShotgunServerScoreRequest(
				HitCharacters,
				Start,
				HitTargets,
				OwnerController->GetServerTime() - OwnerController->SingleTripTime
			);
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleFlashSocket)
	{
		return;
	}

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandomVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLocation = SphereCenter + RandomVector;
		FVector ToEndLocation = EndLocation - TraceStart;
		ToEndLocation = FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
		
		HitTargets.Add(ToEndLocation);
	}
}
