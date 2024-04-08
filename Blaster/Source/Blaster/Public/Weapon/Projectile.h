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

	/*
		use for server-side rewind
	*/
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVeclocity; //want a accurate direction of speed

	UPROPERTY(EditDefaultsOnly)
		float InitialSpeed = 15000.f;

private:
	UPROPERTY(EditDefaultsOnly)
		UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComponent = nullptr;

	FTimerHandle DestroyTimer;
	UPROPERTY(EditDefaultsOnly)
		float DestroyTime = 3.f;

public:
	void SetUseServerSideRewind(bool ShouldUse) { bUseServerSideRewind = ShouldUse; }
	bool GetUseServerSideRewind()const { return bUseServerSideRewind; }
	void SetTraceStart(FVector_NetQuantize Start) { TraceStart = Start; }
	FVector_NetQuantize GetTraceStart()const { return TraceStart; }
	void SetInitialVelocity(FVector_NetQuantize100 Velocity) { InitialVeclocity = Velocity; }
	FVector_NetQuantize100 GetInitialVelocity()const { return InitialVeclocity; }
	float GetInitialSpeed()const { return InitialSpeed; }
	void SetInitialSpeed(float Speed) { InitialSpeed = Speed; }
	float GetBaseDamage()const { return Damage; }
	void SetBaseDamage(float BaseDamage) { Damage = BaseDamage; }
};
