// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/TeamsGameMode.h"
#include "CaptureFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ACaptureFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(
		class ABlasterCharacter* EliminatedCharacter,
		class ABlasterPlayerController* VictimController,
		ABlasterPlayerController* AttackerController
	)override;

	void FlagCaptured(class AFlag* Flag, class AFlagZone* FlagZone);
};
