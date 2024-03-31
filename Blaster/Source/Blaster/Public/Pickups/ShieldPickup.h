// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "ShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShieldPickup : public APickupBase
{
	GENERATED_BODY()
protected:
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& HitResult
	);

private:
	UPROPERTY(EditDefaultsOnly)
		float ShieldReplenishAmount = 100.f;

	UPROPERTY(EditDefaultsOnly)
		float ShieldReplenishTime = 5.f;

};
