// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AbilitySystem/Abilities/AuraGameplayAbility.h"


void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : Abilities)
	{
		FGameplayAbilitySpec GameplayAbilitySpec(AbilityClass, 1);
		if (UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(GameplayAbilitySpec.Ability))
		{
			GameplayAbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
			GiveAbility(GameplayAbilitySpec);
			//GiveAbilityAndActivateOnce(GameplayAbilitySpec);	
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) { return; }

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if(AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if(!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) { return; }

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if(AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* ASC,
                                                const FGameplayEffectSpec& GameplayEffectSpec,
                                                FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	FGameplayTagContainer TagContainer;
	GameplayEffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}
