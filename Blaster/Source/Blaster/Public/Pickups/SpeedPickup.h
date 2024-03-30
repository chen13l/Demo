// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "SpeedPickup.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API ASpeedPickup : public APickupBase
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
	);

private:
	UPROPERTY(EditDefaultsOnly)
		float SpeedBuffTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
		float BaseSpeedBuff = 1000.f;

	UPROPERTY(EditDefaultsOnly)
		float CrouchSpeedBuff = 700.f;
};
