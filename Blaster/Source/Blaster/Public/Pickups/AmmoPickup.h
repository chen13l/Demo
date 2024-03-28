// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AAmmoPickup : public APickupBase
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& HitResult
	)override;

private:
	UPROPERTY(EditDefaultsOnly)
		int32 AmmoAmount = 30;

	UPROPERTY(EditDefaultsOnly)
		EWeaponType WeaponType;
};
