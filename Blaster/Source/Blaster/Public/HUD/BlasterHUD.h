// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage {
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairLeft;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;

	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	class UCharacterOverlay* CharacterOverlay;

private:
	FHUDPackage HUDPackage;
	void DrawCrosshairs(UTexture2D* Tuxture, FVector2D ViewportCenter, FVector2D CrosshairSpread, FLinearColor CrosshairColor);

	UPROPERTY(EditDefaultsOnly)
		float CrosshairSpreadMax = 16;

protected:
	virtual void BeginPlay() override;
	void AddCharacterOverlay();

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }


};
