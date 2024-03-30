// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuffComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	void HealRamup(float DeltaTime);

private:
	UPROPERTY()
		class ABlasterCharacter* BlasterCharacter;
	//heal buff
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;
	//speed buff
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitilaBaseSpeed;
	float InitialCrouchSpeed;
	UFUNCTION(NetMulticast, Reliable)
		void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);
	//jump buff
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity=0.f;
	UFUNCTION(NetMulticast,Reliable)
	void MulticastJumpBuff(float JumpVelocity);
public:
	void SetBlasterCharacter(ABlasterCharacter* Character) { BlasterCharacter = Character; }

	void Heal(float HealAmount, float HealTime);

	void SpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);

	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void SetInitialJumpVelocty(float Velocity);

};
