// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/BlasterCharacter.h"
#include "PlayerController/BlasterPlayerController.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()
		;
	UPROPERTY()
		FVector Location;

	UPROPERTY()
		FRotator Rotation;

	UPROPERTY()
		FVector BoxExtent;

};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()
		;
	UPROPERTY()
		float Time;

	UPROPERTY()
		TMap<FName, FBoxInformation>HitBoxInfo;

	UPROPERTY()
		ABlasterCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()
		;
	UPROPERTY()
		bool bHitCinfirm;

	UPROPERTY()
		bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()
		;
	UPROPERTY()
		TMap<ABlasterCharacter*, uint32>HeadShots;

	UPROPERTY()
		TMap<ABlasterCharacter*, uint32>BodyShots;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable)
		void ServerScoreRequest(
			ABlasterCharacter* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize& HitLocation,
			float HitTime,
			class AWeaponBase* DamagerCauser
		);

	//overload ServerScoreRequest() for Shotgun
	UFUNCTION(Server, Reliable)
		void ShotgunServerScoreRequest(
			const TArray<ABlasterCharacter*>& HitCharacters,
			const FVector_NetQuantize& TraceStart,
			const TArray<FVector_NetQuantize>& HitLocations,
			float HitTime
		);

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);

	FServerSideRewindResult ServerSideRewind(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);
	//overload for shotgun
	FShotgunServerSideRewindResult ServerSideRewind(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);

	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);

	FFramePackage InterBetweenFrame(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTme);

	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation
	);
	//overload for shotgun
	FShotgunServerSideRewindResult ConfirmHit(
		const TArray<FFramePackage>& Packages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
		);

	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	void ResetMoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnable);
	void SaveFramePackage();

	/*
		Shotgun
	*/

private:
	UPROPERTY()
		ABlasterCharacter* BlasterCharacter;

	UPROPERTY()
		ABlasterPlayerController* BlasterController;

	TDoubleLinkedList<FFramePackage>FrameHistory;
	UPROPERTY(EditDefaultsOnly)
		float MaxRecoedTime = 4.f;

	UPROPERTY(EditDefaultsOnly)
		bool bShowFramePackage = false;

public:
	void SetBlasterCharacter(ABlasterCharacter* Character) { BlasterCharacter = Character; }
	void SetBlasterController(ABlasterPlayerController* Controller) { BlasterController = Controller; }

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);
};
