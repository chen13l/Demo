// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileBullet();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event)override;
#endif

protected:
	virtual void BeginPlay()override;

	virtual void OnHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalInpulse,
		const FHitResult& HitResult
	) override;

};
