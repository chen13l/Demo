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
		virtual void OnHit(
			UPrimitiveComponent* HitComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			FVector NormalInpulse,
			const FHitResult& HitResult
		);

	void SpawnTrailSystem();

	void StartDestoryTimer();
	void DestroyTimerFinished();
	void ExplodeDamage();

	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* ImpactParticle;
	UPROPERTY(EditDefaultsOnly)
		class USoundCue* ImpactSound;

	UPROPERTY(EditDefaultsOnly)
		float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly)
		class USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnyWhere)
		class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditDefaultsOnly)
		class UNiagaraSystem* TrailSystem;
	UPROPERTY()
		class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(EditDefaultsOnly)
		float DamageInnerRadius = 200.f;
	UPROPERTY(EditDefaultsOnly)
		float DamageOuterRadius = 500.f;

private:
	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComponent = nullptr;

	FTimerHandle DestroyTimer;
	UPROPERTY(EditDefaultsOnly)
		float DestroyTime = 3.f;

public:


};
