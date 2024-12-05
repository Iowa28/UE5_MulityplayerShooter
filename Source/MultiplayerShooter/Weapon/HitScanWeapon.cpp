// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerShooter/Character/BaseCharacter.h"
#include "MultiplayerShooter/Components/LagCompensationComponent.h"
#include "MultiplayerShooter/Controller/BasePlayerController.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) { return; }
	AController* InstigatorController = OwnerPawn->GetController();
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		if (FireHit.bBlockingHit)
		{
			ABaseCharacter* Character = Cast<ABaseCharacter>(FireHit.GetActor());
			if (Character && InstigatorController && OwnerPawn->IsLocallyControlled())
			{
				if (HasAuthority())
				{
					UGameplayStatics::ApplyDamage(
						Character,
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
				else if (!HasAuthority() && bUseServerSideRewind)
				{
					OwnerCharacter = OwnerCharacter ? OwnerCharacter : Cast<ABaseCharacter>(OwnerPawn);
					OwnerController = OwnerController ? OwnerController : Cast<ABasePlayerController>(InstigatorController);
					if (OwnerCharacter && OwnerController && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
					{
						OwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(
							Character,
							Start,
							HitTarget,
							OwnerController->GetServerTime() - OwnerController->SingleTripTime,
							this
						);
					}
				}
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
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
			}
		}
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	const UWorld* World = GetWorld();
	if (!World) { return; }
	
	const FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
	World->LineTraceSingleByChannel(OutHit, TraceStart, End, ECC_Visibility);
	
	FVector BeamEnd = End;
	if (OutHit.bBlockingHit)
	{
		BeamEnd = OutHit.ImpactPoint;
	}

	DrawDebugSphere(GetWorld(), BeamEnd, 16, 12, FColor::Orange, true);

	if (BeamParticles)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
			World,
			BeamParticles,
			TraceStart,
			FRotator::ZeroRotator,
			true
		);
		if (Beam)
		{
			Beam->SetVectorParameter(FName("Target"), BeamEnd);
		}
	}
}
