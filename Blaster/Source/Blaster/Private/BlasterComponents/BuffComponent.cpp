#include "BlasterComponents/BuffComponent.h"
#include "Character/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();


}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRamup(DeltaTime);
	ShieldRamup(DeltaTime);
}

void UBuffComponent::HealRamup(float DeltaTime)
{
	if (!bHealing || BlasterCharacter == nullptr || BlasterCharacter->GetIsElim()) { return; }

	const float HealThisFame = HealingRate * DeltaTime;
	BlasterCharacter->SetHealth(FMath::Clamp(BlasterCharacter->GetHealth() + HealThisFame, 0, BlasterCharacter->GetMaxHealth()));
	BlasterCharacter->UpdateHUDHealth();
	AmountToHeal -= HealThisFame;

	if (AmountToHeal <= 0 || BlasterCharacter->GetHealth() >= BlasterCharacter->GetMaxHealth()) {
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::Heal(float HealAmount, float HealTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::ShieldRamup(float DeltaTime)
{
	if (!bReplenishShield || BlasterCharacter == nullptr || BlasterCharacter->GetIsElim()) { return; }

	const float ReplenishThisFame = ShieldReplenishRate * DeltaTime;
	BlasterCharacter->SetHealth(FMath::Clamp(BlasterCharacter->GetHealth() + ReplenishThisFame, 0, BlasterCharacter->GetMaxShield()));
	BlasterCharacter->UpdateHUDShield();
	ShieldAmountReplenish -= ReplenishThisFame;

	if (ShieldAmountReplenish <= 0 || BlasterCharacter->GetShield() >= BlasterCharacter->GetMaxShield()) {
		bReplenishShield = false;
		ShieldAmountReplenish = 0.f;
	}
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldAmountReplenish += ShieldAmount;
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitilaBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (BlasterCharacter == nullptr) { return; }
	BlasterCharacter->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&ThisClass::ResetSpeed,
		BuffTime
	);

	if (BlasterCharacter->GetCharacterMovement()) {
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);

}

void UBuffComponent::ResetSpeed()
{
	if (BlasterCharacter == nullptr || BlasterCharacter->GetCharacterMovement() == nullptr) { return; }

	BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = InitilaBaseSpeed;
	BlasterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	MulticastSpeedBuff(InitilaBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (BlasterCharacter && BlasterCharacter->GetCharacterMovement()) {
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

void UBuffComponent::SetInitialJumpVelocty(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::ResetJump()
{
	if (BlasterCharacter == nullptr || BlasterCharacter->GetCharacterMovement() == nullptr) { return; }

	BlasterCharacter->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (BlasterCharacter == nullptr) { return; }
	BlasterCharacter->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&ThisClass::ResetJump,
		BuffTime
	);

	if (BlasterCharacter->GetCharacterMovement()) {
		BlasterCharacter->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (BlasterCharacter && BlasterCharacter->GetCharacterMovement()) {
		BlasterCharacter->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}