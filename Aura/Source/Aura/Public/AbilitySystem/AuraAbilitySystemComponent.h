// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, FGameplayTagContainer& /*Asset Tags*/)

/**
 * 
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	FEffectAssetTags EffectAssetTags;
	
	void AbilityActorInfoSet();
	
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities);
protected:
	void EffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& GameplayEffectSpec,
	                   FActiveGameplayEffectHandle ActiveGameplayEffectHandle);

	
};
