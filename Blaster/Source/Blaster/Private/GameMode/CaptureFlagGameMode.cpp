// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/CaptureFlagGameMode.h"
#include "Weapon/Flag.h"
#include "CaptureFlag/FlagZone.h"
#include "GameState/BlasterGameState.h"

void ACaptureFlagGameMode::PlayerEliminated(
	ABlasterCharacter* EliminatedCharacter,
	ABlasterPlayerController* VictimController,
	ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
}

void ACaptureFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* FlagZone)
{
	bool bValidCapture = Flag->GetTeam() != FlagZone->GetTeam();
	if (bValidCapture) {
		ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(GameState);
		if (BlasterGameState) {
			if (FlagZone->GetTeam() == ETeam::ET_RedTeam)
			{
				BlasterGameState->UpdateRedTeamScore();
			}
			if (FlagZone->GetTeam() == ETeam::ET_BlueTeam)
			{
				BlasterGameState->UpdateBlueTeamScore();
			}
		}
	}
}
