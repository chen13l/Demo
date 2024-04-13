// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TeamsGameMode.h"
#include "GameState/BlasterGameState.h"
#include "PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/BlasterPlayerController.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

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

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ABlasterPlayerState* AttackerPState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) { return BaseDamage; }
	if (AttackerPState == VictimPState) { return BaseDamage; }
	if (AttackerPState->GetTeam() == VictimPState->GetTeam()) { return 0.f; }
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(
	class ABlasterCharacter* EliminatedCharacter,
	class ABlasterPlayerController* VictimController,
	ABlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* AttackerPS = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	if (BGameState && AttackerPS) {
		if (AttackerPS->GetTeam() == ETeam::ET_BlueTeam) {
			BGameState->UpdateBlueTeamScore();
		}
		if (AttackerPS->GetTeam() == ETeam::ET_RedTeam) {
			BGameState->UpdateRedTeamScore();
		}
	}
}