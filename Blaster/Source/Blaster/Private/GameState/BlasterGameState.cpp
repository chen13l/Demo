// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerState/BlasterPlayerState.h"
#include "PlayerController/BlasterPlayerController.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ThisClass, RedTeamScore);
	DOREPLIFETIME(ThisClass, BlueTeamScore);
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer) {
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer) {
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ABlasterGameState::UpdateRedTeamScore()
{
	++RedTeamScore;
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer) {
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::UpdateBlueTeamScore()
{
	++BlueTeamScore;
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer) {
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
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
