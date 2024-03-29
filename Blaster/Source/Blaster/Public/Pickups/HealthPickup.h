// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "HealthPickup.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickupBase
{
	GENERATED_BODY()

public:
	AHealthPickup();
	virtual void Destroyed()override;
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
		float HealthAmount = 100.f;

	UPROPERTY(EditDefaultsOnly)
		float HealthingTime = 5.f;

	UPROPERTY(VisibleAnywhere)
		class UNiagaraComponent* PickupEffectComponent;

	UPROPERTY(EditDefaultsOnly)
		class UNiagaraSystem* PickupEffect;

};
