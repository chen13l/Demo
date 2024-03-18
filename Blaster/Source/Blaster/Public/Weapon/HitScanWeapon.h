// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponBase.h"
#include "HitScanWeapon.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeaponBase
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget)override;


private:
	UPROPERTY(EditDefaultsOnly)
		float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly)
		class UParticleSystem* ImpactParticle;
};
