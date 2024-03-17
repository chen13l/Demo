// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();
	virtual void Destroyed()override;
protected:
	virtual void BeginPlay()override;

	virtual void OnHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalInpulse,
		const FHitResult& HitResult
	)override;

private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* RocketMesh;

	UPROPERTY(EditDefaultsOnly)
		class UNiagaraSystem* TrailSystem;

	UPROPERTY()
		class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(EditDefaultsOnly)
		class USoundCue* ProjectileLoop;

	UPROPERTY()
		class UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditDefaultsOnly)
		class USoundAttenuation* LoopSoundAttenuation;


	FTimerHandle DestroyTimer;
	UPROPERTY(EditDefaultsOnly)
		float DestroyTime = 3.f;
	void DestroyTimerFinished();
};
