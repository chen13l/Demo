// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"

void ABlasterGameMode::PlayerEliminated(
	class ABlasterCharacter* EliminatedCharacter,
	class ABlasterPlayerController* VictimComtroller,
	ABlasterPlayerController* AttackerController)
{
	if (EliminatedCharacter) {
		EliminatedCharacter->Eliminated();
	}
}