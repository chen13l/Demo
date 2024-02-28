// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/WeaponBase.h"
#include "BlasterComponents/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent) {
		CombatComponent->BlasterCharacter = this;
	}
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
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

			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
			EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ThisClass::EquipButtonPressed);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::OnPressedCrouch);
			EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Triggered, this, &ThisClass::OnPressedAiming);
			EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Completed, this, &ThisClass::OnReleaseAiming);
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
