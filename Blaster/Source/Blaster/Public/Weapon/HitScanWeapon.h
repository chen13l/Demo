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

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

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

	/*
		TraceEnd with scatter
	*/

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Scatter")
		float DistanceToSphere = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Scatter")
		float SphereRadius = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Scatter")
		bool bUseScatter = false;

};
