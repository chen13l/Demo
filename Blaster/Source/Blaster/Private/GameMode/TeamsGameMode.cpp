// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TeamsGameMode.h"
#include "GameState/BlasterGameState.h"
#include "PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState) {
		for (auto PState : BGameState->PlayerArray) {
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam) {
				if (BGameState->GetRedTeam().Num() >= BGameState->GetBlueTeam().Num()) {
					BGameState->GetBlueTeam().AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
				else {
					BGameState->GetRedTeam().AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
			}
		}
	}
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState) {
		ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam) {
			if (BGameState->GetRedTeam().Num() >= BGameState->GetBlueTeam().Num()) {
				BGameState->GetBlueTeam().AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
			else {
				BGameState->GetRedTeam().AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState) {
		ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>();
		if (BPState) {
			if (BGameState->GetRedTeam().Contains(BPState)) {
				BGameState->GetRedTeam().Remove(BPState);
			}
			if (BGameState->GetBlueTeam().Contains(BPState)) {
				BGameState->GetBlueTeam().Remove(BPState);
			}
		}
	}
}