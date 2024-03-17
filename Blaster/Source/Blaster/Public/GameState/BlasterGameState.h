// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

	void UpdateTopScore(class ABlasterPlayerState* TopPlayerState);

	TArray<ABlasterPlayerState*> GetTopPlayers()const { return TopScoringPlayers; }

private:
	UPROPERTY(Replicated)
		TArray<ABlasterPlayerState*> TopScoringPlayers;

	float TopScore = 0.f;
};
