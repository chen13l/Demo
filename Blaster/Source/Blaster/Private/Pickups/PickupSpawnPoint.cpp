#include "Pickups/PickupSpawnPoint.h"
#include "Pickups/PickupBase.h"
#include "TimerManager.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
	StartPickupSpawnTimer((AActor*)nullptr);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupSpawnPoint::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses > 0) {
		UWorld* World = GetWorld();
		if (World) {
			int32 RandIndex = FMath::RandRange(0, NumPickupClasses - 1);
			SpawnedPickup = World->SpawnActor<APickupBase>(PickupClasses[RandIndex], GetActorTransform());

			if (HasAuthority() && SpawnedPickup) {
				SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartPickupSpawnTimer);
			}
		}
	}
}

void APickupSpawnPoint::StartPickupSpawnTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::RandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(
		SpawnPickupTimer,
		this,
		&APickupSpawnPoint::SpawnPickupTimerFinished,
		SpawnTime
	);
}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority()) {
		SpawnPickup();
	}
}
