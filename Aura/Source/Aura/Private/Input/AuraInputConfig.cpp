// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/AuraInputConfig.h"

const UInputAction* UAuraInputConfig::GetInputActionByTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const auto [InputAction,InputActionTag] : AbilityInputActions)
	{
		if (InputAction && InputActionTag.MatchesTagExact(InputTag))
		{
			return InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction fot InputTag [%s] in InputConfig [%s}"),
		       *InputTag.GetTagName().ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
