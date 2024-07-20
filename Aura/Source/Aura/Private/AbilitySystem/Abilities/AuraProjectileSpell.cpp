// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	
}

void UAuraProjectileSpell::SpawnProjectile()
{
	const bool bInServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if(!bInServer){return;}
	
	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());

	if (CombatInterface)
	{
		//TODO: set SpawnRotation
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(CombatInterface->GetCombatSocketLocation());
		AAuraProjectile* SpawnActorDeferred = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass, SpawnTransform, GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		//TODO: handle damage

		SpawnActorDeferred->FinishSpawning(SpawnTransform);
	}
}
