// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "MultiplayerShooter/Components/CombatComponent.h"
#include "MultiplayerShooter/Controller/BasePlayerController.h"
#include "MultiplayerShooter/GameMode/ShooterGameMode.h"
#include "MultiplayerShooter/PlayerState/BasePlayerState.h"
#include "MultiplayerShooter/Weapon/Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 720.f);
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	Health = MaxHealth;
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABaseCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABaseCharacter, Health);
	DOREPLIFETIME(ABaseCharacter, bDisableGameplay);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ThisClass::EquipButtonPressed);

		EnhancedInputComponent->BindAction(DuckAction, ETriggerEvent::Started, this, &ThisClass::DuckButtonPressed);
		EnhancedInputComponent->BindAction(DuckAction, ETriggerEvent::Completed, this, &ThisClass::DuckButtonReleased);

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ThisClass::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::AimButtonReleased);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::FireButtonReleased);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ThisClass::ReloadButtonPressed);
	}
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ABaseCharacter::PollInit()
{
	if (!CharacterPlayerState)
	{
		CharacterPlayerState = GetPlayerState<ABasePlayerState>();
		if (CharacterPlayerState)
		{
			CharacterPlayerState->AddToScore(0.f);
			CharacterPlayerState->AddToDefeats(0.f);
		}
	}
}

void ABaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void ABaseCharacter::Destroyed()
{
	Super::Destroyed();

	if (EliminationComponent)
	{
		EliminationComponent->DestroyComponent();
	}

	const AShooterGameMode* GameMode = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
	const bool bMatchNotInProgress = GameMode && GameMode->GetMatchState() != MatchState::InProgress;
	if (CombatComponent && CombatComponent->EquippedWeapon && bMatchNotInProgress)
	{
		CombatComponent->EquippedWeapon->Destroy();
	}
}

#pragma region Movement
void ABaseCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0;
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
	if (Controller && !bDisableGameplay)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		const FVector2D MovementVector = Value.Get<FVector2D>();
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
	if (Controller)
	{
		const FVector2D LookAxisVector = Value.Get<FVector2D>();
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ABaseCharacter::Jump()
{
	if (bDisableGameplay) { return; }
	
	if (bIsCrouched)
	{
		UnCrouch();
	}
	
	Super::Jump();
}

void ABaseCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) { return; }

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}
#pragma endregion Movement

#pragma region Duck
void ABaseCharacter::DuckButtonPressed()
{
	if (bDisableGameplay) { return; }
	Crouch();
}

void ABaseCharacter::DuckButtonReleased()
{
	UnCrouch();
}
#pragma endregion Duck

#pragma region Aiming
void ABaseCharacter::AimButtonPressed()
{
	if (CombatComponent && !bDisableGameplay)
	{
		CombatComponent->SetAiming(true);
	}
}

void ABaseCharacter::AimButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}
#pragma endregion Aiming

#pragma region AimOffset
void ABaseCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > .25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAimOffsetPitch();
	}
}

void ABaseCharacter::AimOffset(float DeltaTime)
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon) { return; }
	
	const float Speed = CalculateSpeed();
	const bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (FMath::IsNearlyEqual(Speed, 0.f) && !bIsInAir)
	{
		bRotateRootBone = true;
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AimOffsetYaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAimOffsetYaw = AimOffsetYaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	else if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AimOffsetYaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAimOffsetPitch();
}

float ABaseCharacter::CalculateSpeed() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return  Velocity.Size();
}

void ABaseCharacter::CalculateAimOffsetPitch()
{
	AimOffsetPitch = GetBaseAimRotation().Pitch;
	if (!IsLocallyControlled() && AimOffsetPitch > 90.f)
	{
		const FVector2D InRange = FVector2D(270.f, 360.f);
		const FVector2D OutRange = FVector2D(-90.f, 0.f);
		AimOffsetPitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimOffsetPitch);
	}
}

void ABaseCharacter::SimProxiesTurn()
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon) { return; }

	bRotateRootBone = false;
	
	const float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (ProxyYaw > TurnThreshold)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (ProxyYaw < -TurnThreshold)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	else
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
}

void ABaseCharacter::TurnInPlace(float DeltaTime)
{
	if (AimOffsetYaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AimOffsetYaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAimOffsetYaw = FMath::FInterpTo(InterpAimOffsetYaw, 0.f, DeltaTime, 5.f);
		AimOffsetYaw = InterpAimOffsetYaw;
		if (FMath::Abs(AimOffsetYaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}
#pragma endregion AimOffset

#pragma region Health/Damage
void ABaseCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                   AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMathf::Max(0, Health - Damage);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (FMath::IsNearlyEqual(Health, 0))
	{
		if (AShooterGameMode* ShooterGameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>())
		{
			BasePlayerController = BasePlayerController ? BasePlayerController : Cast<ABasePlayerController>(GetController());
			ABasePlayerController* AttackerController = Cast<ABasePlayerController>(InstigatorController);
			ShooterGameMode->PlayerEliminated(this, BasePlayerController, AttackerController);
		}
	}
}

void ABaseCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void ABaseCharacter::UpdateHUDHealth()
{
	BasePlayerController = BasePlayerController ? BasePlayerController : Cast<ABasePlayerController>(GetController());
	if (BasePlayerController)
	{
		BasePlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABaseCharacter::PlayHitReactMontage()
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon || bEliminated) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		const FName SectionName =FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
#pragma endregion Health/Damage

#pragma region Elimination
void ABaseCharacter::PlayEliminationMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EliminationMontage)
	{
		AnimInstance->Montage_Play(EliminationMontage);
	}
}

void ABaseCharacter::Eliminate()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Dropped();
	}
	MulticastEliminate();
	GetWorldTimerManager().SetTimer(
		EliminationTimer,
		this,
		&ThisClass::EliminationTimerFinished,
		EliminationDelay
	);
}

void ABaseCharacter::MulticastEliminate_Implementation()
{
	if (bEliminated) { return; }

	if (BasePlayerController)
	{
		BasePlayerController->SetHUDWeaponAmmo(0);
	}
	
	bEliminated = true;
	PlayEliminationMontage();

	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), .55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
		StartDissolve();
	}

	GetCharacterMovement()->DisableMovement();
	bDisableGameplay = true;
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (EliminationEffect)
	{
		FVector EliminationSpawnPoint = GetActorLocation();
		EliminationSpawnPoint.Z += 200.f;
		EliminationComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			EliminationEffect,
			EliminationSpawnPoint,
			GetActorRotation()
		);
	}

	if (EliminationSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			EliminationSound,
			GetActorLocation()
		);
	}
}

void ABaseCharacter::EliminationTimerFinished()
{
	if (AShooterGameMode* ShooterGameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>())
	{
		ShooterGameMode->RequestRespawn(this, GetController());
	}
}

void ABaseCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABaseCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}
#pragma endregion Elimination

#pragma region Fire
void ABaseCharacter::FireButtonPressed()
{
	if (CombatComponent && !bDisableGameplay)
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void ABaseCharacter::FireButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void ABaseCharacter::PlayFireMontage(bool bAiming)
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireMontage)
	{
		AnimInstance->Montage_Play(FireMontage);
		const FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
#pragma endregion Fire

#pragma region Reload
void ABaseCharacter::ReloadButtonPressed()
{
	if (CombatComponent && !bDisableGameplay)
	{
		CombatComponent->Reload();
	}
}

void ABaseCharacter::PlayReloadMontage()
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
			break;
		}
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
#pragma endregion Reload

#pragma region Equipment
void ABaseCharacter::EquipButtonPressed()
{
	if (CombatComponent && !bDisableGameplay)
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void ABaseCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}
#pragma endregion Equipment

#pragma region OverlappingWeapon
void ABaseCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}


void ABaseCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}
#pragma endregion OverlappingWeapon

#pragma region Getters
bool ABaseCharacter::IsWeaponEquipped() const
{
	return CombatComponent && CombatComponent->EquippedWeapon;
}

bool ABaseCharacter::IsAiming() const
{
	return CombatComponent && CombatComponent->bAiming;
}

AWeapon* ABaseCharacter::GetEquippedWeapon() const
{
	return CombatComponent ? CombatComponent->EquippedWeapon : nullptr;
}

FVector ABaseCharacter::GetHitTarget() const
{
	return CombatComponent ? CombatComponent->HitTarget : FVector();
}

ECombatState ABaseCharacter::GetCombatState() const
{
	return CombatComponent ? CombatComponent->CombatState : ECombatState::ECS_MAX;
}
#pragma endregion Getters
