// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerState/BlasterPlayerState.h"
#include "GameState/BlasterGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	//game state will stay in WaitingToStart until call StartMatch()
	bDelayedStart = true;
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart) {
		CountdownTime = WarmupTime + LevelStartingTime - GetWorld()->GetTimeSeconds();
		if (CountdownTime <= 0.f) {
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress) {
		CountdownTime = WarmupTime + MatchTime + LevelStartingTime - GetWorld()->GetTimeSeconds();
		if (CountdownTime <= 0.f) {
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown) {
		CountdownTime = WarmupTime + MatchTime + CooldownTime + LevelStartingTime - GetWorld()->GetTimeSeconds();
		if (CountdownTime <= 0.f) {
			RestartGame();
		}
	}
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator IT = GetWorld()->GetPlayerControllerIterator(); IT; ++IT) {
		ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*IT);
		if (BlasterPlayerController) {
			BlasterPlayerController->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(
	class ABlasterCharacter* EliminatedCharacter,
	class ABlasterPlayerController* VictimController,
	ABlasterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
	
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState) {
		AttackerPlayerState->AddToScore(1.f);
		ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
		if (BlasterGameState) {
			BlasterGameState->UpdateTopScore(AttackerPlayerState);
		}
	}
	if (VictimPlayerState) {
		VictimPlayerState->AddToDefeats(1);
	}

	if (EliminatedCharacter) {
		EliminatedCharacter->Elim();
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimCharacter, AController* ElimController)
{
	if (ElimCharacter) {
		ElimCharacter->Reset();
		ElimCharacter->Destroy();
	}
	if (ElimController) {
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 PlayerStartsIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimController, PlayerStarts[PlayerStartsIndex]);
	}
}
