// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterCharacter.h"
#include "Weapon/WeaponBase.h"
#include "BlasterComponents/CombatComponent.h"
#include "BlasterComponents/BuffComponent.h"
#include "Blaster/Blaster.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "PlayerController/BlasterPlayerController.h"
#include "GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "PlayerState/BlasterPlayerState.h"
#include "Weapon/WeaponTypes.h"


// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = true;
	//fix respawn issue
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 350.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTiemlineComponent"));

	AttachGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent) {
		CombatComponent->BlasterCharacter = this;
	}
	if (BuffComponent) {
		BuffComponent->SetBlasterCharacter(this);
		BuffComponent->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed,GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocty(GetCharacterMovement()->JumpZVelocity);
	}
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();

	if (HasAuthority()) {
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
	if (AttachGrenade) {
		AttachGrenade->SetVisibility(false);
	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr) {
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState) {
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
		}
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent) {
		ElimBotComponent->DestroyComponent();
	}

	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMathNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (CombatComponent && CombatComponent->EquippedWeapon && bMathNotInProgress) {
		CombatComponent->EquippedWeapon->Destroyed();
	}
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotateInPlace(DeltaTime);
	HideCharacterWhenTooClose();
	PollInit();
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay) {
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled()) {
		AimOffset(DeltaTime);
	}
	else {
		SimProxiesTurn();
		TimeSinceLastMovementRep += DeltaTime;
		if (TimeSinceLastMovementRep > 0.25f) {
			OnRep_ReplicatedMovement();
		}
		CaculateAO_Pitch();
	}
}

void ABlasterCharacter::HideCharacterWhenTooClose()
{
	if (!IsLocallyControlled() || FollowCamera == nullptr) { return; }
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold) {
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh()) {
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else {
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh()) {
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float ABlasterCharacter::CaculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) { return; }

	float Speed = CaculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) {
		bShouldRotateRootBone = true;
		FRotator CurrentRotation(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, BaseRotation);
		AO_Yaw = DeltaRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning) {
			InterpYaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;

		TurnInPlace(DeltaTime);
	}
	if (Speed != 0.f || bIsInAir) {
		bShouldRotateRootBone = false;
		BaseRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CaculateAO_Pitch();
}

void ABlasterCharacter::CaculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	//在数据通过网络发送前会进行压缩,变为无符号即[0,360)
	if (AO_Pitch > 90.f && !IsLocallyControlled()) {
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) { return; }

	bShouldRotateRootBone = false;
	float Speed = CaculateSpeed();
	if (Speed > 0) {
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	if (FMath::Abs(ProxyYaw) > TurnThreshold) {
		if (ProxyYaw > TurnThreshold) {
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold) {
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else {
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f) {
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f) {
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning) {
		InterpYaw = FMath::FInterpTo(InterpYaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpYaw;
		if (FMath::Abs(AO_Yaw) < 15.f) {
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			BaseRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementRep = 0.f;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}


// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent) {
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if (InputSubsystem) {
			InputSubsystem->AddMappingContext(IMC_MovementBase, 0);
			if (!bDisableGameplay) {
				EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
				EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
				EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Jump);
				EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
				EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ThisClass::EquipButtonPressed);
				EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::OnPressedCrouch);
				EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Triggered, this, &ThisClass::OnPressedAiming);
				EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Completed, this, &ThisClass::OnReleaseAiming);
				EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ThisClass::OnFiredButtonPressed);
				EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::OnFiredButtonRelease);
				EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ThisClass::OnReloadButtonPressed);
				EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Triggered, this, &ThisClass::OnGreandeButtonPressed);
			}
			else {
				EnhancedInputComponent->ClearActionBindings();
				EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
			}
		}
	}

}

void ABlasterCharacter::Move(const FInputActionValue& Value) {
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector ForwardDirection(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		const FVector RigthDirection(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RigthDirection, MovementVector.X);
	}
}

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();

	if (Controller != nullptr) {
		AddControllerYawInput(LookVector.X);
		AddControllerPitchInput(LookVector.Y);
	}
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (CombatComponent) {
		if (HasAuthority()) {
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else {
			ServerEquipButtonPressed();
		}
	}
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched) {
		UnCrouch();
	}
	else {
		Super::Jump();
	}
}

void ABlasterCharacter::OnPressedCrouch()
{
	if (bIsCrouched) {
		UnCrouch();
	}
	else {
		Crouch();
	}
}

void ABlasterCharacter::OnPressedAiming()
{
	if (CombatComponent) {
		CombatComponent->SetAiming(true);
	}
}

void ABlasterCharacter::OnReleaseAiming()
{
	if (CombatComponent) {
		CombatComponent->SetAiming(false);
	}
}

void ABlasterCharacter::OnFiredButtonPressed()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->OnFiredButtonPressed(true);
	}
}

void ABlasterCharacter::OnFiredButtonRelease()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->OnFiredButtonPressed(false);
	}
}

void ABlasterCharacter::OnReloadButtonPressed()
{
	if (CombatComponent) {
		CombatComponent->Reload();
	}

}

void ABlasterCharacter::OnGreandeButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->ThrowGrenade();
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage) {
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
		//PlayAnimMontage(FireWeaponMontage, 1.f, SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage) {
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
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		default:
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage) {
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
		//PlayAnimMontage(FireWeaponMontage, 1.f, SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage) {
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage) {
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialinstance) {
		DynamicDissolveMaterialinstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline) {
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeaponBase* LastWeapon)
{
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon) {
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent) {
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeaponBase* Weapon)
{
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled()) {
		if (OverlappingWeapon) {
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsEquippedWeapon()const {
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming() const
{
	return (CombatComponent && CombatComponent->bIsAiming);
}

AWeaponBase* ABlasterCharacter::GetEquippedWeapon() const
{
	return CombatComponent == nullptr ? nullptr : CombatComponent->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (CombatComponent == nullptr)return FVector();
	return CombatComponent->HitTarget;
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth) {
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController) {
		BlasterPlayerController->SetHealthPercent(Health, MaxHealth);
	}
}

void ABlasterCharacter::ReceiveDamage(
	AActor* DamagedActor,
	float Damage,
	const UDamageType* DamageType,
	AController* InstigatorController,
	AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.f) {
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode) {
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::Elim()
{
	if (CombatComponent && CombatComponent->EquippedWeapon) {
		CombatComponent->EquippedWeapon->Dropped();
	}

	MulticastElim();

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ThisClass::ElimTimerFinished,
		ElimDelay
	);
}

void ABlasterCharacter::MulticastElim_Implementation()
{
	if (BlasterPlayerController) {
		BlasterPlayerController->SetHUDAmmo(0);
	}

	bIsElim = true;
	PlayElimMontage();

	//Start dissolve effect
	if (DissolveMaterialInstance) {
		DynamicDissolveMaterialinstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialinstance);
		DynamicDissolveMaterialinstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialinstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	//disable movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	SetDisableGameplay(true);
	GetCharacterMovement()->DisableMovement();

	//disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//spawn elim bot
	if (ElimBotEffect) {
		FVector ElimBotSpawnPoint(GetActorLocation());;
		ElimBotSpawnPoint.Z += 200.f;

		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			this,
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}

	if (ElimBotSound) {
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
	bool bHideScopeWidget =
		CombatComponent &&
		CombatComponent->EquippedWeapon &&
		CombatComponent->bIsAiming &&
		CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideScopeWidget) {
		ShowScopeWidget(false);
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode) {
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

ECombatState ABlasterCharacter::GetCombatState()const
{
	if (CombatComponent == nullptr) { return ECombatState::ECS_MAX; }

	return CombatComponent->GetCombatState();
}

void ABlasterCharacter::SetDisableGameplay(bool ShouldDisalbe)
{
	bDisableGameplay = ShouldDisalbe;
	if (CombatComponent)
	{
		CombatComponent->SetWantFire(false);
	}
	if (InputComponent) {
		SetupPlayerInputComponent(InputComponent);
	}
}

void ABlasterCharacter::OnRep_DisableGameplay()
{
	if (CombatComponent)
	{
		CombatComponent->SetWantFire(false);
	}
	if (InputComponent) {
		SetupPlayerInputComponent(InputComponent);
	}
}