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
	void EquipPrimaryWeapon(AWeaponBase* WeaponToEquip);
	void EquipSecondaryWeapon(AWeaponBase* WeaponToEquip);
	void SwapWeapons();
	void DropEquippedWeapon();

	void AttachActorToRightHand(AActor* AttachActor);
	void AttachActorToLeftHand(AActor* AttachActor);
	void AttachActorToBackpack(AActor* AttachActor);

	void PlayEquipWeaponSound(AWeaponBase* WeaponToEquip);

	void UpdateCarriedAmmo();
	void AutoReloadWeapon();

	void SetAiming(bool bAiming);
	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bAiming);

	void OnFiredButtonPressed(bool bPressed);
	void Fire();
	void FireHitScanWeapon();
	void FireProjectileWeapon();
	void FireShotgun();
	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(Server, Reliable)
		void ServerShotgunFire(const TArray< FVector_NetQuantize>& TraceHitTargets);
	UFUNCTION(NetMulticast, Reliable)
		void MulticastShotgunFire(const  TArray< FVector_NetQuantize>& TraceHitTargets);

	//play fire effect before actually receive response from server
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void LocalShotgunFire(const  TArray< FVector_NetQuantize>& TraceHitTargets);

	bool CanFire();

	UFUNCTION(Server, Reliable)
		void ServerReload();
	void HandleReload();

	void TraceUnderCrossHair(FHitResult& TraceResult);

	/*
		Grenade
	*/
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class AProjectile> GrenadeClass;

	void ThrowGrenade();
	UFUNCTION(Server, Reliable)
		void ServerThrowGrenade();
	UFUNCTION(BlueprintCallable)
		void ThrowGrenadeFinished();

	void ShowAttachGrenade(bool ShouldShowGrenade);
	UFUNCTION(BlueprintCallable)
		void LaunchGrenade();
	UFUNCTION(Server, Reliable)
		void ServerLaunchGrenade(const FVector_NetQuantize& Target);

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
	UFUNCTION()
		void OnRep_EquippedWeapon();

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
		AWeaponBase* SecondaryWeapon;
	UFUNCTION()
		void OnRep_SecondaryWeapon();

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
		bool bIsAiming = false;
	UFUNCTION()
		void OnRep_Aiming();
	bool bAimButtomPressed = false;

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
		TMap<EWeaponType, int32>StartingCarriedAmmo;

	void InitializeCarruedAmmo(EWeaponType WeaponType);

	int32 AmountToReload();
	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

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

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
		int32 Grenades = 4;
	UFUNCTION()
		void OnRep_Grenades();

	void UpdateHUDGrenades();

	UPROPERTY(EditDefaultsOnly)
		int32 MaxGrenades = 4;

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

	/*
		default weapon
	*/
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AWeaponBase>DefaultWeaponClass;

	void InitializeStartingHUDAmmo();

	UFUNCTION(BlueprintCallable)
		void FinishReloading();
	bool bLocallyReloading = false;
public:
	void SetHUDCrosshair(float DeltaTime);

	void Reload();
	bool GetIsLocallyReload()const { return bLocallyReloading; }

	UFUNCTION(BlueprintCallable)
		void ShotgunShellReload();
	void JumpToSectionEnd();

	ECombatState GetCombatState()const { return CombatState; }
	void SetWantFire(bool WantFire) { bWantFire = WantFire; }

	FORCEINLINE int32 GetGrenades() const { return Grenades; }

	bool CanSwapWeapon() { return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr && CombatState == ECombatState::ECS_Unoccupied); }
	/*
		pickup
	*/
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	/*
		Default Weapon
	*/
	void SpawnDefautlWeapon();

	AWeaponBase* GetSecondaryWeapon()const { return SecondaryWeapon; }
};
