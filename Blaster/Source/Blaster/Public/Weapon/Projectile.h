// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UParticleSystem;

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
		void OnHit(
			UPrimitiveComponent* HitComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			FVector NormalInpulse,
			const FHitResult& HitResult
		);

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* ImpactParticle;
	UPROPERTY(EditDefaultsOnly)
		class USoundCue* ImpactSound;

private:
	UPROPERTY(EditDefaultsOnly)
		class USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnyWhere)
		class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComponent;

public:


};
