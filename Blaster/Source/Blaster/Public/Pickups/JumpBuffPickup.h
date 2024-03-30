// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "JumpBuffPickup.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AJumpBuffPickup : public APickupBase
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
		float JumpZVelocityBuff = 1500.f;
	UPROPERTY(EditDefaultsOnly)
		float JumpBuffTime = 5.f;
};
