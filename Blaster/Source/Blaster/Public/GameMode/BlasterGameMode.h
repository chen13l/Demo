// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime)override;

	ABlasterGameMode();
	virtual void PlayerEliminated(
		class ABlasterCharacter* EliminatedCharacter,
		class ABlasterPlayerController* VictimController,
		ABlasterPlayerController* AttackerController
	);

	virtual void RequestRespawn(class ACharacter* ElimCharacter, class AController* ElimController);

	FORCEINLINE float GetWarmupTime()const { return WarmupTime; }
	FORCEINLINE float GetLevelStartingTime()const { return LevelStartingTime; }
	FORCEINLINE float GetMatchTime() const { return MatchTime; }

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet()override;

private:
	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
		float MatchTime = 120.f;

	float LevelStartingTime = 0.f;
	float CountdownTime = 0.f;

};
