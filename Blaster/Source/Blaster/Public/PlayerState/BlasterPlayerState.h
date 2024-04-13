// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "BlasterPlayerState.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatAmount);

	virtual void OnRep_Score() override;
	UFUNCTION()
		virtual void OnRep_Defeats();

	/*
		team
	*/
	FORCEINLINE ETeam GetTeam()const { return Team; }
	void SetTeam(ETeam TeamToSet);


private:
	class ABlasterCharacter* Character = nullptr;
	class ABlasterPlayerController* Controller = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
		int32 Defeats;

	/*
		team
	*/
	UPROPERTY(ReplicatedUsing = OnRep_Team)
		ETeam Team = ETeam::ET_NoTeam;
	UFUNCTION()
		void OnRep_Team();

};
