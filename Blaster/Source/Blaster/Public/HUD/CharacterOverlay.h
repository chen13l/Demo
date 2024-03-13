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
		class UTextBlock* ScoreText;

	UPROPERTY(meta = (BindWiget))
		class UTextBlock* DefeatsText;

	UPROPERTY(meta = (BindWiget))
		class UTextBlock* AmmoAmount;
};
