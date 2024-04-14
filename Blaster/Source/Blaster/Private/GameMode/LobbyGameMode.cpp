// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionSubsystem.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);

	int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance) {
		UMultiplayerSessionSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
		check(Subsystem);

		if (NumOfPlayers == Subsystem->GetDesiredNumPublicConnections()) {
			UWorld* World = GetWorld();
			if (World) {
				bUseSeamlessTravel = true;
				FString MatchTypeString = Subsystem->GetDesiredMatchType();
				if (MatchTypeString == "FreeForAll") {
					World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
				}
				else if (MatchTypeString == "Teams") {
					World->ServerTravel(FString("/Game/Maps/TeamMap?listen"));
				}
				else if (MatchTypeString == "CaptureFlag") {
					World->ServerTravel(FString("/Game/Maps/FlagMap?listen"));
				}
			}
		}
	}
}