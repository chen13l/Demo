// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"


void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	for(TSubclassOf<UGameplayAbility> AbilityClass : Abilities)
	{
		FGameplayAbilitySpec GameplayAbilitySpec(AbilityClass, 1);
		//GiveAbility(GameplayAbilitySpec);
		GiveAbilityAndActivateOnce(GameplayAbilitySpec);
	}
}

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* ASC,
                                                const FGameplayEffectSpec& GameplayEffectSpec,
                                                FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	FGameplayTagContainer TagContainer;
	GameplayEffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
	
	
}
