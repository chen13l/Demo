// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime)override;
	virtual void OnPossess(APawn* InPawn)override;
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	virtual float GetServerTime()const;
	void SetHealthPercent(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDAmmo(int32 WeaponAmmo);
	void SetHUDCarryAmmo(int32 CarryAmmo);
	void SetMatchTime(float Time);
	void SetHUDAnnouncementCountdown(float Countdown);
	void SetHUDGrenade(int32 Grenades);

	void OnMatchStateSet(FName State);
	void HandleCooldown();
protected:
	virtual void BeginPlay() override;
	void PollInit();

	void SetHUDTime();
	void HandleMatchStarted();
	/*
		sync time between client and server
	*/
	//requests the current server time, passing int the client's time when the request was sent
	UFUNCTION(Server, Reliable)
		void ServerRequestServerTime(float TimeOfClientRequest);
	//reports the current server to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
		void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f;

	UPROPERTY(EditDefaultsOnly)
		float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0.f;
	void UpdateTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
		void ServerCheckMatchState();
	UFUNCTION(Client, Reliable)
		void ClientJoinMidGame(FName StateOfTheMatch, float Warmup, float TimeOfMatch, float StartingTime, float Cooldown);

private:
	class ABlasterHUD* BlasterHUD = nullptr;

	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	float LevelStartingTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName StateOfMatch;
	UFUNCTION()
		void OnRep_MatchState();

	UPROPERTY()
		class UCharacterOverlay* CharacterOverlay;
	UPROPERTY()
		class ABlasterGameMode* BlasterGameMode;

	bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;

	int32 HUDGrenades;
};
