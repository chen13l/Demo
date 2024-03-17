// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/BlasterPlayerController.h"
#include "HUD/BlasterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/Announcement.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "BlasterComponents/CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character/BlasterCharacter.h"
#include "GameMode/BlasterGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/BlasterGameMode.h"
#include "GameState/BlasterGameState.h"
#include "PlayerState/BlasterPlayerState.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	UpdateTimeSync(DeltaTime);
	PollInit();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, StateOfMatch);
}

void ABlasterPlayerController::UpdateTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime >= TimeSyncFrequency) {
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter) {
		SetHealthPercent(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController()) {
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHealthPercent(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}

void ABlasterPlayerController::SetHealthPercent(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText;

	if (bHUDValid) {
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		FString HealtText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealtText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreText;

	if (bHUDValid) {
		FString ScoreText = FString::Printf(TEXT("Score: %d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreText->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsText;

	if (bHUDValid) {
		FString DefeatsText = FString::Printf(TEXT("Defeats: %d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsText->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDAmmo(int32 WeaponAmmo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->AmmoAmount;

	if (bHUDValid) {
		FString AmmoText = FString::Printf(TEXT(" %d"), WeaponAmmo);
		BlasterHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDCarryAmmo(int32 CarryAmmo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarryAmmoAmount;

	if (bHUDValid) {
		FString AmmoText = FString::Printf(TEXT(" %d"), CarryAmmo);
		BlasterHUD->CharacterOverlay->CarryAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

float ABlasterPlayerController::GetServerTime()const
{
	if (HasAuthority()) {
		return GetWorld()->GetTimeSeconds();
	}
	else {
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
}

void ABlasterPlayerController::SetMatchTime(float Time)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchTime;

	if (bHUDValid) {
		if (Time < 0.f) {
			BlasterHUD->CharacterOverlay->MatchTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(Time / 60.f);
		int32 Seconds = Time - Minutes * 60;

		FString TimeText = FString::Printf(TEXT(" %02d : %02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchTime->SetText(FText::FromString(TimeText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float Countdown)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->AnnouncementText;

	if (bHUDValid) {
		if (Countdown < 0.f) {
			BlasterHUD->Announcement->AnnouncementText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(Countdown / 60.f);
		int32 Seconds = Countdown - Minutes * 60;

		FString TimeText = FString::Printf(TEXT(" %02d : %02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(TimeText));
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (StateOfMatch == MatchState::InProgress) {
		TimeLeft = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
	}
	else if (StateOfMatch == MatchState::WaitingToStart) {
		TimeLeft = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
	}
	else if (StateOfMatch == MatchState::Cooldown) {
		TimeLeft = WarmupTime + MatchTime + CooldownTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (CountdownInt != SecondsLeft)
	{
		if (StateOfMatch == MatchState::InProgress) {
			SetMatchTime(TimeLeft);
		}
		if (StateOfMatch == MatchState::WaitingToStart || StateOfMatch == MatchState::Cooldown) {
			SetHUDAnnouncementCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceive = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceive);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundtripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundtripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	StateOfMatch = State;
	if (StateOfMatch == MatchState::InProgress) {
		HandleMatchStarted();
	}
	else if (StateOfMatch == MatchState::Cooldown) {
		HandleCooldown();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (StateOfMatch == MatchState::InProgress) {
		HandleMatchStarted();
	}
	else if (StateOfMatch == MatchState::Cooldown) {
		HandleCooldown();
	}
}

void ABlasterPlayerController::HandleMatchStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD) {
		if (BlasterHUD->CharacterOverlay == nullptr) {
			BlasterHUD->AddCharacterOverlay();
		}
		if (BlasterHUD->Announcement) {
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	if (BlasterGameMode) {
		WarmupTime = BlasterGameMode->GetWarmupTime();
		MatchTime = BlasterGameMode->GetMatchTime();
		LevelStartingTime = BlasterGameMode->GetLevelStartingTime();
		StateOfMatch = BlasterGameMode->GetMatchState();
		CooldownTime = BlasterGameMode->GetCooldownTime();
		ClientJoinMidGame(StateOfMatch, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);
	}
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfTheMatch, float Warmup, float TimeOfMatch, float StartingTime, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = TimeOfMatch;
	LevelStartingTime = StartingTime;
	StateOfMatch = StateOfTheMatch;
	CooldownTime = Cooldown;
	OnMatchStateSet(StateOfMatch);
	if (BlasterHUD && StateOfMatch == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD) {
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = BlasterHUD->Announcement &&
			BlasterHUD->Announcement->AnnouncementText &&
			BlasterHUD->Announcement->InfoText;
		if (bHUDValid)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString NewAnnouncementText("New Match Starts in:");
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(NewAnnouncementText));

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			if (BlasterGameState && BlasterPlayerState) {
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->GetTopPlayers();
				FString InfoTextString;
				if (TopPlayers.Num() == 0) {
					InfoTextString = FString("There is no winner");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
				{
					InfoTextString = FString("You are the winner!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for the win:\n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlasterCharacter) {
		BlasterCharacter->SetDisableGameplay(true);
		if (BlasterCharacter->GetCombatComponent())
		{
			BlasterCharacter->GetCombatComponent()->SetWantFire(false);
		}
	}
}
