// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

UCLASS()
class BLASTER_API APickupBase : public AActor
{
	GENERATED_BODY()

public:
	APickupBase();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
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
		class USphereComponent* OverlappedSphere;

	UPROPERTY(EditDefaultsOnly)
		class USoundCue* PickupSound;

	UPROPERTY(EditDefaultsOnly)
		class UStaticMeshComponent* PickupMesh;

public:


};
