// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponBase.h"
#include "Weapon/WeaponTypes.h"
#include "Flag.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AFlag : public AWeaponBase
{
	GENERATED_BODY()

public:
	AFlag();
	virtual void Dropped();

protected:
	virtual void OnEquipped();
	virtual void OnDropped();

private:
	UPROPERTY(EditDefaultsOnly)
		UStaticMeshComponent* FlagMesh;
};
