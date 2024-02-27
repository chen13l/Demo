// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterAnimInstance.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}


void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (BlasterCharacter == nullptr) {
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) { return; }

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	UCharacterMovementComponent* CharacterMovement = BlasterCharacter->GetCharacterMovement();
	bIsInAir = CharacterMovement->IsFalling();
	bIsAccelerating = CharacterMovement->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bWeaponEquipped = BlasterCharacter->IsEquippedWeapon();
	bIsCrouching = CharacterMovement->bWantsToCrouch;
	bIsAiming = BlasterCharacter->IsAiming();
}