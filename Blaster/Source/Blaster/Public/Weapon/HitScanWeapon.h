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

protected:
	UPROPERTY(EditDefaultsOnly)
		float Damage = 20.f;

private:
	UPROPERTY(EditDefaultsOnly)
		class UParticleSystem* ImpactParticle;

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* BeamParticle;

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* MuzzleFlash;

	UPROPERTY(EditDefaultsOnly)
		USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly)
		USoundCue* HitSound;
};
