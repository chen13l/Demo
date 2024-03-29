#include "BlasterComponents/BuffComponent.h"
#include "Character/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"

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
}

void UBuffComponent::HealRamup(float DealtaTime)
{
	if (!bHealing || BlasterCharacter == nullptr || BlasterCharacter->GetIsElim()) { return; }

	const float HealThisFame = HealingRate * DealtaTime;
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
