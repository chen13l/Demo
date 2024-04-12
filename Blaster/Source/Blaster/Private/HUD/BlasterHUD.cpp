// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/Announcement.h"
#include "HUD/ElimAnnouncement.h"
#include "TimerManager.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass) {
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass) {
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::DrawHUD() {
	FVector2D ViewportSize;
	if (GEngine) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	const FVector2D ViewportCenter(ViewportSize.X / 2, ViewportSize.Y / 2);
	const float CrosshairScale = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

	if (HUDPackage.CrosshairCenter) {
		FVector2D Spread(0.f, 0.f);
		DrawCrosshairs(HUDPackage.CrosshairCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
	}
	if (HUDPackage.CrosshairLeft) {
		FVector2D Spread(-CrosshairScale, 0.f);
		DrawCrosshairs(HUDPackage.CrosshairLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
	}
	if (HUDPackage.CrosshairRight) {
		FVector2D Spread(CrosshairScale, 0.f);
		DrawCrosshairs(HUDPackage.CrosshairRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
	}
	if (HUDPackage.CrosshairTop) {
		FVector2D Spread(0.f, -CrosshairScale);
		DrawCrosshairs(HUDPackage.CrosshairTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
	}
	if (HUDPackage.CrosshairBottom) {
		FVector2D Spread(0.f, CrosshairScale);
		DrawCrosshairs(HUDPackage.CrosshairBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
	}
}

void ABlasterHUD::DrawCrosshairs(UTexture2D* Tuxture, FVector2D ViewportCenter, FVector2D CrosshairSpread, FLinearColor CrosshairColor) {
	const float TextureWidth = Tuxture->GetSizeX();
	const float TextureHeight = Tuxture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - TextureWidth / 2.f + CrosshairSpread.X,
		ViewportCenter.Y - TextureHeight / 2.f + CrosshairSpread.Y
	);

	DrawTexture(
		Tuxture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
	);
}

void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;
	if (OwningPlayerController && ElimAnnouncementClass) {
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayerController, ElimAnnouncementClass);
		if (ElimAnnouncementWidget) {
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			ElimAnnouncementWidget->AddToViewport();

			for (UElimAnnouncement* Msg : ElimMsgs) {
				if (Msg && Msg->AnnouncementBox) {
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if (CanvasSlot) {
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(
							CanvasSlot->GetPosition().X, 
							Position.Y - CanvasSlot->GetSize().Y
						);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}

			ElimMsgs.Add(ElimAnnouncementWidget);

			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinsh"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(
				ElimMsgTimer,
				ElimMsgDelegate,
				ElimAnnouncementTime,
				false
			);
		}
	}
}

void ABlasterHUD::ElimAnnouncementTimerFinsh(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove) {
		MsgToRemove->RemoveFromParent();
	}
}