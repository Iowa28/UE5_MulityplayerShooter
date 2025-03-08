// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerShooter/MultiplayerShooter.h"
#include "MultiplayerShooter/Components/BuffComponent.h"
#include "MultiplayerShooter/Components/CombatComponent.h"
#include "MultiplayerShooter/Components/LagCompensationComponent.h"
#include "MultiplayerShooter/Controller/BasePlayerController.h"
#include "MultiplayerShooter/GameMode/ShooterGameMode.h"
#include "MultiplayerShooter/GameState/ShooterGameState.h"
#include "MultiplayerShooter/PlayerState/BasePlayerState.h"
#include "MultiplayerShooter/Weapon/Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// UE_LOG(LogTemp, Warning, TEXT("Sandwich"));
	// UE_LOG(LogTemp, Warning, TEXT("%f"), Variable);

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

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));

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

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetupAttachment(GetMesh(), FName("AttachedGrenade"));

#pragma region HitBoxes
	
	Head = CreateDefaultSubobject<UBoxComponent>(TEXT("Head"));
	Head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("Head"), Head);
	
	Pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("Pelvis"));
	Pelvis->SetupAttachment(GetMesh(), FName("Pelvis"));
	HitCollisionBoxes.Add(FName("Pelvis"), Pelvis);
	
	Spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_02"));
	Spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("Spine_02"), Spine_02);
	
	Spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_03"));
	Spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("Spine_03"), Spine_03);
	
	UpperArm_L = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArm_L"));
	UpperArm_L->SetupAttachment(GetMesh(), FName("UpperArm_L"));
	HitCollisionBoxes.Add(FName("UpperArm_L"), UpperArm_L);
	
	UpperArm_R = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArm_R"));
	UpperArm_R->SetupAttachment(GetMesh(), FName("UpperArm_R"));
	HitCollisionBoxes.Add(FName("UpperArm_R"), UpperArm_R);
	
	LowerArm_L = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArm_L"));
	LowerArm_L->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("LowerArm_L"), LowerArm_L);
	
	LowerArm_R = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArm_R"));
	LowerArm_R->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("LowerArm_R"), LowerArm_R);
	
	Hand_L = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_L"));
	Hand_L->SetupAttachment(GetMesh(), FName("Hand_L"));
	HitCollisionBoxes.Add(FName("Hand_L"), Hand_L);
	
	Hand_R = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_R"));
	Hand_R->SetupAttachment(GetMesh(), FName("Hand_R"));
	HitCollisionBoxes.Add(FName("Hand_R"), Hand_R);
	
	Backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("Backpack"));
	Backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("Backpack"), Backpack);
	
	Blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("Blanket"));
	Blanket->SetupAttachment(GetMesh(), FName("blanket_l"));
	HitCollisionBoxes.Add(FName("Blanket"), Blanket);
	
	Thigh_L = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_L"));
	Thigh_L->SetupAttachment(GetMesh(), FName("Thigh_L"));
	HitCollisionBoxes.Add(FName("Thigh_L"), Thigh_L);
	
	Thigh_R = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_R"));
	Thigh_R->SetupAttachment(GetMesh(), FName("Thigh_R"));
	HitCollisionBoxes.Add(FName("Thigh_R"), Thigh_R);
	
	Calf_L = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_L"));
	Calf_L->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("Calf_L"), Calf_L);
	
	Calf_R = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_R"));
	Calf_R->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("Calf_R"), Calf_R);
	
	Foot_L = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_L"));
	Foot_L->SetupAttachment(GetMesh(), FName("Foot_L"));
	HitCollisionBoxes.Add(FName("Foot_L"), Foot_L);
	
	Foot_R = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_R"));
	Foot_R->SetupAttachment(GetMesh(), FName("Foot_R"));
	HitCollisionBoxes.Add(FName("Foot_R"), Foot_R);

	for (const TTuple<FName, UBoxComponent*> Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
	
#pragma endregion HitBoxes
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABaseCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABaseCharacter, Health);
	DOREPLIFETIME(ABaseCharacter, Shield);
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

	Health = MaxHealth;
	Shield = 0;
	UpdateHUDHealth();
	UpdateHUDShield();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}

	AttachedGrenade->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), TEXT("GrenadeSocket"));
	AttachedGrenade->SetVisibility(false);
	SpawnDefaultWeapon();
	UpdateHUDAmmo();
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ThisClass::EquipButtonPressed);
		EnhancedInputComponent->BindAction(SwapAction, ETriggerEvent::Started, this, &ThisClass::SwapButtonPressed);

		EnhancedInputComponent->BindAction(DuckAction, ETriggerEvent::Started, this, &ThisClass::DuckButtonPressed);
		EnhancedInputComponent->BindAction(DuckAction, ETriggerEvent::Completed, this, &ThisClass::DuckButtonReleased);

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ThisClass::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::AimButtonReleased);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::FireButtonReleased);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ThisClass::ReloadButtonPressed);
		
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Started, this, &ThisClass::ThrowGrenadeButtonPressed);
	}
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCharacterIfCameraIsClose();
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
			SetTeamColor(CharacterPlayerState->GetTeam());

			const AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
			if (ShooterGameState && ShooterGameState->TopScoringPlayers.Contains(CharacterPlayerState))
			{
				MulticastGainedTheLead();
			}
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
	if (BuffComponent)
	{
		BuffComponent->Character = this;
		BuffComponent->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensationComponent)
	{
		LagCompensationComponent->Character = this;
		if (Controller)
		{
			LagCompensationComponent->Controller = Cast<ABasePlayerController>(Controller);
		}
	}
}

void ABaseCharacter::Destroyed()
{
	Super::Destroyed();

	if (EliminationComponent)
	{
		EliminationComponent->DestroyComponent();
	}

	GameMode = GameMode ? GameMode : GetWorld()->GetAuthGameMode<AShooterGameMode>();
	const bool bMatchNotInProgress = GameMode && GameMode->GetMatchState() != MatchState::InProgress;
	if (CombatComponent && CombatComponent->EquippedWeapon && bMatchNotInProgress)
	{
		CombatComponent->EquippedWeapon->Destroy();
	}
}

void ABaseCharacter::SetTeamColor(const ETeam Team)
{
	if (!GetMesh() || !DefaultMaterial || !BlueMaterial || !RedMaterial) { return; }
	
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, DefaultMaterial);
		DissolveMaterialInstance = BlueDissolveMaterialInstance;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMaterialInstance;
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMaterialInstance;
		break;
	}
}

void ABaseCharacter::MulticastGainedTheLead_Implementation()
{
	if (!CrownSystem) { return; }

	if (!CrownComponent)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetMesh(),
			FName(),
			GetActorLocation() + FVector(0, 0, 110),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	else
	{
		CrownComponent->Activate();
	}
}

void ABaseCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
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

void ABaseCharacter::HideCharacterIfCameraIsClose()
{
	if (!IsLocallyControlled()) { return; }

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (CombatComponent && CombatComponent->SecondaryWeapon && CombatComponent->SecondaryWeapon->GetWeaponMesh())
		{
			CombatComponent->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (CombatComponent && CombatComponent->SecondaryWeapon && CombatComponent->SecondaryWeapon->GetWeaponMesh())
		{
			CombatComponent->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
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
	if (CombatComponent && CombatComponent->bHoldingTheFlag)
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
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
	GameMode = GameMode ? GameMode : GetWorld()->GetAuthGameMode<AShooterGameMode>();
	if (bEliminated || !GameMode) { return; }

	float DamageToHealth = GameMode->CalculateDamage(InstigatorController, Controller, Damage);
	
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Min(Shield - Damage, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Min(DamageToHealth - Shield, Damage);
			Shield = 0.f;
		}
		UpdateHUDShield();
	}
	
	Health = FMathf::Max(0, Health - DamageToHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (FMath::IsNearlyEqual(Health, 0))
	{
		BasePlayerController = BasePlayerController ? BasePlayerController : Cast<ABasePlayerController>(GetController());
		ABasePlayerController* AttackerController = Cast<ABasePlayerController>(InstigatorController);
		GameMode->PlayerEliminated(this, BasePlayerController, AttackerController);
	}
}

void ABaseCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABaseCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABaseCharacter::UpdateHUDHealth()
{
	BasePlayerController = BasePlayerController ? BasePlayerController : Cast<ABasePlayerController>(GetController());
	if (BasePlayerController)
	{
		BasePlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABaseCharacter::UpdateHUDShield()
{
	BasePlayerController = BasePlayerController ? BasePlayerController : Cast<ABasePlayerController>(GetController());
	if (BasePlayerController)
	{
		BasePlayerController->SetHUDShield(Shield, MaxShield);
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

void ABaseCharacter::Eliminate(bool bPlayerLeftGame)
{
	if (CombatComponent)
	{
		DropOrDestroyWeapon(CombatComponent->EquippedWeapon);
		DropOrDestroyWeapon(CombatComponent->SecondaryWeapon);
	}
	MulticastEliminate(bPlayerLeftGame);
}

void ABaseCharacter::MulticastEliminate_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;	
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
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
		UGameplayStatics::SpawnSoundAtLocation(this, EliminationSound, GetActorLocation());
	}
	
	if (IsLocallyControlled() && CombatComponent && CombatComponent->bAiming && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ToggleSniperScopeWidget(false);
	}

	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}

	GetWorldTimerManager().SetTimer(EliminationTimer, this, &ThisClass::EliminationTimerFinished,EliminationDelay);
}

void ABaseCharacter::EliminationTimerFinished()
{
	GameMode = GameMode ? GameMode : GetWorld()->GetAuthGameMode<AShooterGameMode>();
	if (GameMode && !bLeftGame)
	{
		GameMode->RequestRespawn(this, GetController());
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ABaseCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (!Weapon) { return; }

	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
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

void ABaseCharacter::ServerLeaveGame_Implementation()
{
	GameMode = GameMode ? GameMode : GetWorld()->GetAuthGameMode<AShooterGameMode>();
	CharacterPlayerState = CharacterPlayerState ? CharacterPlayerState : GetPlayerState<ABasePlayerState>();
	if (GameMode && CharacterPlayerState)
	{
		GameMode->PlayerLeftGame(CharacterPlayerState);
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
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SMG:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
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
		if (CombatComponent->CombatState == ECombatState::ECS_Unoccupied)
		{
			ServerEquipButtonPressed();
		}
		const bool bSwap = CombatComponent->ShouldSwapWeapons()
			&& !HasAuthority()
			&& CombatComponent->CombatState == ECombatState::ECS_Unoccupied
			&& !OverlappingWeapon;
		if (bSwap)
		{
			PlaySwapWeaponMontage();
			CombatComponent->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}

void ABaseCharacter::SwapButtonPressed()
{
	if (CombatComponent && !bDisableGameplay)
	{
		if (CombatComponent->CombatState == ECombatState::ECS_Unoccupied)
		{
			ServerSwapButtonPressed();
		}
		const bool bSwap = CombatComponent->ShouldSwapWeapons()
			&& !HasAuthority()
			&& CombatComponent->CombatState == ECombatState::ECS_Unoccupied;
		if (bSwap)
		{
			PlaySwapWeaponMontage();
			CombatComponent->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}

void ABaseCharacter::ServerEquipButtonPressed_Implementation()
{
	if (!CombatComponent) { return; }
	
	if (OverlappingWeapon)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
	else if (CombatComponent->ShouldSwapWeapons())
	{
		CombatComponent->SwapWeapons();
	}
}

void ABaseCharacter::ServerSwapButtonPressed_Implementation()
{
	if (CombatComponent && CombatComponent->ShouldSwapWeapons())
	{
		CombatComponent->SwapWeapons();
	}
}

void ABaseCharacter::PlaySwapWeaponMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapWeaponsMontage)
	{
		AnimInstance->Montage_Play(SwapWeaponsMontage);
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

#pragma region ThrowGrenade
void ABaseCharacter::ThrowGrenadeButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->ThrowGrenade();
	}
}

void ABaseCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}
#pragma endregion ThrowGrenade

#pragma region DefaultWeapon

void ABaseCharacter::SpawnDefaultWeapon()
{
	GameMode = GameMode ? GameMode : GetWorld()->GetAuthGameMode<AShooterGameMode>();
	UWorld* World = GetWorld();
	if (GameMode && World && !bEliminated && DefaultWeaponClass && CombatComponent)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		CombatComponent->EquipWeapon(StartingWeapon);
	}
}

void ABaseCharacter::UpdateHUDAmmo()
{
	BasePlayerController = BasePlayerController ? BasePlayerController : Cast<ABasePlayerController>(GetController());
	if (BasePlayerController && CombatComponent && CombatComponent->EquippedWeapon)
	{
		BasePlayerController->SetHUDCarriedAmmo(CombatComponent->CarriedAmmo);
		BasePlayerController->SetHUDWeaponAmmo(CombatComponent->EquippedWeapon->GetAmmo());
	}
}

#pragma endregion DefaultWeapon

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

bool ABaseCharacter::IsLocallyReloading() const
{
	return CombatComponent && CombatComponent->bLocallyReloading;
}

bool ABaseCharacter::IsHoldingTheFlag() const
{
	return CombatComponent && CombatComponent->bHoldingTheFlag;
}

#pragma endregion Getters
