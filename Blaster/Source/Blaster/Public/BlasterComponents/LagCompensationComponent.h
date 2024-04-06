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
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);

private:
	UPROPERTY()
		ABlasterCharacter* BlasterCharacter;

	UPROPERTY()
		ABlasterPlayerController* BlasterController;

public:
	void SetBlasterCharacter(ABlasterCharacter* Character) { BlasterCharacter = Character; }
	void SetBlasterController(ABlasterPlayerController* Controller) { BlasterController = Controller; }

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);
};
