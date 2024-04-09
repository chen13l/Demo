// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	FHighPingDelegate HighPingDelegate;

	virtual void Tick(float DeltaTime)override;
	virtual void OnPossess(APawn* InPawn)override;
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	virtual float GetServerTime()const;
	float GetSingleTripTime()const { return SingleRoundtripTime; }
	void SetHealthPercent(float Health, float MaxHealth);
	void SetShieldPercent(float Shield, float MaxShield);
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

	/*
		Ping
	*/
	void CheckPing(float DeltaTime);
	void HighPingWarning();
	void StopHighPingWarning();

private:
	class ABlasterHUD* BlasterHUD = nullptr;

	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	float LevelStartingTime = 0.f;
	uint32 CountdownInt = 0;

	float SingleRoundtripTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName StateOfMatch;
	UFUNCTION()
		void OnRep_MatchState();

	UPROPERTY()
		class UCharacterOverlay* CharacterOverlay;
	UPROPERTY()
		class ABlasterGameMode* BlasterGameMode;

	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializehealth = false;

	float HUDShield;
	float HUDMaxShield;
	bool bInitializeShield = false;

	float HUDScore;
	bool bInitializeScore = false;

	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	int32 HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	int32 HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

	/*
		ping
	*/
	UPROPERTY(EditDefaultsOnly)
		float HighPingRunningTime = 0.f;
	UPROPERTY(EditDefaultsOnly)
		float PingAnimRunningTime = 0.f;
	UPROPERTY(EditDefaultsOnly)
		float HighPingDuration = 5.f;
	UPROPERTY(EditDefaultsOnly)
		float CheckPingFrequency = 20.f;
	UPROPERTY(EditDefaultsOnly)
		float HighPingThreshold = 50.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);
};
