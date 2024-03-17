// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/BlasterPlayerState.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* TopPlayerState)
{
	if (TopScoringPlayers.Num() == 0) {
		TopScoringPlayers.Add(TopPlayerState);
		TopScore = TopPlayerState->GetScore();
	}
	else if (TopPlayerState->GetScore() == TopScore) {
		TopScoringPlayers.AddUnique(TopPlayerState);
	}
	else if (TopPlayerState->GetScore() > TopScore) {
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(TopPlayerState);
		TopScore = TopPlayerState->GetScore();
	}
}
