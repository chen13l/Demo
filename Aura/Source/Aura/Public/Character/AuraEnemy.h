// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "AuraEnemy.generated.h"

/**
 *
 */
UCLASS()
class AURA_API AAuraEnemy : public ACharacterBase, public IEnemyInterface
{
	GENERATED_BODY()

public:
	AAuraEnemy();
	/*  EnemyInterface  */
	virtual void HighLight() override;
	virtual void UnHighLight() override;
	/*  End EnemyInterface  */

	/*Combat Interface*/
	virtual int32 GetPlayerLevel() const override;
	/*End Combat Interface*/
	
	
protected:
	virtual void BeginPlay() override;

	virtual void InitAbilityActorinfo() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character Class Defaults", meta=(AllowPrivateAccess))
	int32 Level = 1;
}
;
