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
	class UCharacterOverlay* CharacterOverlay = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
		TSubclassOf<UUserWidget> AnnouncementClass;
	class UAnnouncement* Announcement = nullptr;

private:
	class APlayerController* OwningPlayerController;

	FHUDPackage HUDPackage;
	void DrawCrosshairs(UTexture2D* Tuxture, FVector2D ViewportCenter, FVector2D CrosshairSpread, FLinearColor CrosshairColor);

	UPROPERTY(EditDefaultsOnly)
		float CrosshairSpreadMax = 16;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class UElimAnnouncement>ElimAnnouncementClass;
	UPROPERTY(EditDefaultsOnly)
		float ElimAnnouncementTime = 2.5f;
	UPROPERTY()
		TArray<UElimAnnouncement*> ElimMsgs;
	UFUNCTION()
		void ElimAnnouncementTimerFinsh(UElimAnnouncement* MsgToRemove);

protected:
	virtual void BeginPlay() override;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

	void AddCharacterOverlay();
	void AddAnnouncement();
	void AddElimAnnouncement(FString Attacker, FString Victim);

};
