// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	void SpawnPickup();

private:
	UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<class APickupBase>>PickupClasses;

	UPROPERTY(EditDefaultsOnly)
		float SpawnPickupTimeMin = 1.f;
	UPROPERTY(EditDefaultsOnly)
		float SpawnPickupTimeMax = 5.f;
	FTimerHandle SpawnPickupTimer;
	APickupBase* SpawnedPickup;

	UFUNCTION()
		void StartPickupSpawnTimer(AActor* DestroyedActor);

	void SpawnPickupTimerFinished();

public:

};