// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
		class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWiget))
		class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWiget))
		UTextBlock* ScoreText;

	UPROPERTY(meta = (BindWiget))
		UTextBlock* DefeatsText;

	UPROPERTY(meta = (BindWiget))
		UTextBlock* AmmoAmount;

	UPROPERTY(meta = (BindWiget))
		UTextBlock* CarryAmmoAmount;

	UPROPERTY(meta = (BindWiget))
		UTextBlock* MatchTime;
};
