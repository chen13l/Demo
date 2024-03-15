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
	virtual float GetServerTime()const;
	void SetHealthPercent(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDAmmo(int32 WeaponAmmo);
	void SetHUDCarryAmmo(int32 CarryAmmo);
	void SetMatchTime(float Time);
protected:
	virtual void BeginPlay() override;

	void SetHUDTime();

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

private:
	class ABlasterHUD* BlasterHUD = nullptr;

	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
};
