// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/BlasterHUD.h"
#include "Weapon/WeaponTypes.h"
#include "Weapon/WeaponBase.h"
#include "Blaster/BlasterTypes/CombatState.h"
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
	UFUNCTION()
		void OnRep_EquippedWeapon();

	void SetAiming(bool bAiming);
	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bAiming);

	void OnFiredButtonPressed(bool bPressed);
	void Fire();
	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	bool CanFire();

	UFUNCTION(Server, Reliable)
		void ServerReload();

	void HandleReload();

	void TraceUnderCrossHair(FHitResult& TraceResult);

private:
	class ABlasterCharacter* BlasterCharacter = nullptr;
	class ABlasterPlayerController* BlasterController = nullptr;
	/*
		Movement
	*/
	float BaseWalkkSpeed;
	float AimWalkSpeed;

	/*
		Combat
	*/
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
		AWeaponBase* EquippedWeapon;

	UPROPERTY(Replicated)
		bool bIsAiming;

	FVector HitTarget;

	bool bWantFire;
	//for fire mode
	bool bCanFire = true;
	FTimerHandle FireTimer;
	void StartFireTimer();
	void EndFireTimer();

	//ammo for currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarryAmmo)
		int32 CarryAmmo;
	UFUNCTION()
		void OnRep_CarryAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditDefaultsOnly)
		int32 StartingCarriedAmmo = 30.f;

	void InitializeCarruedAmmo(EWeaponType WeaponType);

	int32 AmountToReload();
	void UpdateAmmoValues();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
		ECombatState CombatState = ECombatState::ECS_Unoccupied;
	UFUNCTION()
		void OnRep_CombatState();

	/*
		HUDand Crosshair
	*/
	class ABlasterHUD* BlasterHUD = nullptr;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FHUDPackage HUDPackage;

	/*
	* Aimingand FOV
	*/
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
	void Reload();
	UFUNCTION(BlueprintCallable)
		void FinishReloading();
	ECombatState GetCombatState()const { return CombatState; }
	void SetWantFire(bool WantFire) { bWantFire = WantFire; }
};
