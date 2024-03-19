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
	virtual void WeaponTraceHit(const FVector& HitStart, const FVector& HitTarget, FHitResult& OutHit);
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	UPROPERTY(EditDefaultsOnly)
		float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly)
		USoundCue* HitSound;

	UPROPERTY(EditDefaultsOnly)
		class UParticleSystem* ImpactParticle;
private:
	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* BeamParticle;

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* MuzzleFlash;

	UPROPERTY(EditDefaultsOnly)
		USoundCue* FireSound;

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
