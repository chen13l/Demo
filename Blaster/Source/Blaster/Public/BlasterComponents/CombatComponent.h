// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeaponBase;

#define TRACE_LENGTH 80000;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps)const override;
	friend class ABlasterCharacter;
protected:
	virtual void BeginPlay() override;
	void EquipWeapon(AWeaponBase* WeaponToEquip);

	void SetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bAiming);

	UFUNCTION()
		void OnRep_EquippedWeapon();

	void OnFiredButtonPressed(bool bPressed);
	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrossHair(FHitResult& TraceResult);

private:
	class ABlasterCharacter* BlasterCharacter;
	class ABlasterPlayerController* BlasterController;
	class ABlasterHUD* BlasterHUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
		AWeaponBase* EquippedWeapon;

	UPROPERTY(Replicated)
		bool bIsAiming;

	float BaseWalkkSpeed;
	float AimWalkSpeed;
	bool bWantFire;

	//HUD and Crosshair
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	//Aiming and FOV

	//FOV when not aiming; set to the camera's base FOV in Beginplay
	float DefaultFOV;
	float CurrentFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		float ZoomedFOV = 30.f;
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		float ZoomInterpSpeed = 20.f;
	void InterpFOV(float DeltaTime);


public:
	void SetHUDCrosshair(float DeltaTime);
	
};
