// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameFramework/Actor.h"
#include "AuraEffectActor.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class EEffectApplicationPolicy
{
	AppliedBeginOverlap,
	AppliedEndOverlap,
	NotApplied
};

UENUM(BlueprintType)
enum class EEffectRemovalPolicy
{
	RemoveEndOverlap,
	NotRemove
};

UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()

public:
	AAuraEffectActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

	UFUNCTION(BlueprintCallable)
	void OnBeginOverlap(AActor* TargetActor);
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Applied Effects")
	TSubclassOf<UGameplayEffect> AppliedGameplayEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Applied Effects")
	EEffectApplicationPolicy EffectApplicationPolicy = EEffectApplicationPolicy::NotApplied;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Applied Effects")
	EEffectRemovalPolicy EffectRemovalPolicy = EEffectRemovalPolicy::RemoveEndOverlap;
	
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles_ASC;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Applied Effects")
	float ActorLevel = 1.f;
	
};
