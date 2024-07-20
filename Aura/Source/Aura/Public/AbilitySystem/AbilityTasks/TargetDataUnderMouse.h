// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataSignature, const FGameplayAbilityTargetDataHandle&, Data);

/**
 * use TargetData need to go AuraAssetManager call UAbilitySystemGlobals::Get().InitGlobalData()
 */
UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks",
		meta=(DisplayName = "Target Date Under Mouse", HidePin="OwningAbility", DefaultToSelf="OwningAbility",
			BlueprintInternalUseOnly = true)
	)
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);

	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSignature TargetDataDelegate;

private:
	virtual void Activate() override;
	void SendTargetData();

	UFUNCTION()
	void OnTargetDataReplicationCallback(const FGameplayAbilityTargetDataHandle& DataHandle,
	                                     FGameplayTag ActivationTag);
};
