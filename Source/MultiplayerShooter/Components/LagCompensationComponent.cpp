// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/Character/BaseCharacter.h"
#include "MultiplayerShooter/Controller/BasePlayerController.h"
#include "MultiplayerShooter/Weapon/Weapon.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SaveFramePackage();
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (!Character || !Character->HasAuthority()) { return; }
	
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		return;
	}

	float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
	while (HistoryLength > MaxRecordTime)
	{
		FrameHistory.RemoveNode(FrameHistory.GetTail());
		HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
	}
	
	FFramePackage ThisFrame;
	SaveFramePackage(ThisFrame);
	FrameHistory.AddHead(ThisFrame);
	// ShowFramePackage(ThisFrame, FColor::Magenta);
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, FColor Color)
{
	for (const TTuple<FName, FBoxInformation>& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			BoxInfo.Value.Rotation.Quaternion(),
			Color,
			false,
			4
		);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character ? Character : Cast<ABaseCharacter>(GetOwner());
	if (!Character) { return; }

	Package.Time = GetWorld()->GetTimeSeconds();
	for (TTuple<FName, UBoxComponent*>& BoxPair : Character->HitCollisionBoxes)
	{
		FBoxInformation BoxInformation;
		BoxInformation.Location = BoxPair.Value->GetComponentLocation();
		BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
		BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
		Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
	}
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABaseCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	if (!Character) { return; }
	// if (!Controller && Character->Controller)
	// {
	// 	Controller = Cast<ABasePlayerController>(Controller);
	// }
	
	const FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (HitCharacter && DamageCauser && Character && Confirm.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			DamageCauser->GetDamage(),
			Character->Controller,
			DamageCauser,
			UDamageType::StaticClass()
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABaseCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	if (!HitCharacter || !HitCharacter->GetLagCompensationComponent()) { return FServerSideRewindResult(); }

	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	if (!History.GetTail() || !History.GetHead()) { return FServerSideRewindResult(); }

	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	if (OldestHistoryTime > HitTime) { return FServerSideRewindResult(); }

	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	
	if (FMath::IsNearlyEqual(OldestHistoryTime, HitTime))
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime < HitTime || FMath::IsNearlyEqual(NewestHistoryTime, HitTime))
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	while (Older->GetValue().Time > HitTime)
	{
		if (!Older->GetNextNode()) { break; }
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if (FMath::IsNearlyEqual(Older->GetValue().Time, HitTime))
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	if (bShouldInterpolate)
	{
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpSpeed = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0, 1);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (const TTuple<FName, FBoxInformation>& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1, InterpSpeed);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1, InterpSpeed);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}
	
	return InterpFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABaseCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	FServerSideRewindResult Result;
	if (!Character) { return Result; }

	FFramePackage CurrentFrame;
	CacheBoxPosition(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("Head")];
	if (!HeadBox) { return Result; }
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	FHitResult ConfirmHitResult;
	if (UWorld* World = GetWorld())
	{
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_Visibility
		);
		if (ConfirmHitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			Result.bHitConfirmed = true;
			Result.bHeadshot = true;
		}
		else
		{
			for (TTuple<FName, UBoxComponent*>& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_Visibility
			);
			if (ConfirmHitResult.bBlockingHit)
			{
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				Result.bHitConfirmed = true;
			}
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return Result;
}

void ULagCompensationComponent::CacheBoxPosition(ABaseCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (!Character) { return; }

	for (TTuple<FName, UBoxComponent*>& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABaseCharacter* HitCharacter, const FFramePackage& Package)
{
	if (!HitCharacter) { return; }

	for (TTuple<FName, UBoxComponent*>& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value)
		{
			const FBoxInformation BoxInfo = Package.HitBoxInfo[HitBoxPair.Key];
			HitBoxPair.Value->SetWorldLocation(BoxInfo.Location);
			HitBoxPair.Value->SetWorldRotation(BoxInfo.Rotation);
			HitBoxPair.Value->SetBoxExtent(BoxInfo.BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABaseCharacter* HitCharacter, const FFramePackage& Package)
{
	if (!HitCharacter) { return; }

	for (TTuple<FName, UBoxComponent*>& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value)
		{
			const FBoxInformation BoxInfo = Package.HitBoxInfo[HitBoxPair.Key];
			HitBoxPair.Value->SetWorldLocation(BoxInfo.Location);
			HitBoxPair.Value->SetWorldRotation(BoxInfo.Rotation);
			HitBoxPair.Value->SetBoxExtent(BoxInfo.BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABaseCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (!HitCharacter || !HitCharacter->GetMesh()) { return; }
	HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
}
