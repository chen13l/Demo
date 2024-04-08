// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponBase.h"
#include "ProjectileWeapon.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeaponBase
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget)override;

private:
	//replicates
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class AProjectile> ProjectileClass;

	//no replicates
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
};
