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

	/*
		team
	*/
	TArray<ABlasterPlayerState*>RedTeam;
	TArray<ABlasterPlayerState*>BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
		float RedTeamScore = 0.f;
	UFUNCTION()
		void OnRep_RedTeamScore();
	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
		float BlueTeamScore = 0.f;
	UFUNCTION()
		void OnRep_BlueTeamScore();


public:
	/*
		Team
	*/
	TArray<ABlasterPlayerState*>GetRedTeam() { return RedTeam; }
	TArray<ABlasterPlayerState*>GetBlueTeam() { return BlueTeam; }
	float GetRedTeamScore()const { return RedTeamScore; }
	float GetBlueTeamScore()const { return BlueTeamScore; }
	void UpdateRedTeamScore();
	void UpdateBlueTeamScore();
};
