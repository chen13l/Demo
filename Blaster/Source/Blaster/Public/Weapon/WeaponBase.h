// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/WeaponTypes.h"
#include "WeaponBase.generated.h"

UENUM(BlueprintType)
enum class EWeaponState :uint8 {
	EWS_Initial UMETA(DisplayName = "Initail State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "EquippedSecondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType :uint8
{
	EFT_HitScan UMETA(DisplayName = "HitScanWeapon"),
	EFT_Projectile UMETA(DisplayName = "ProjectileWeapon"),
	EFT_Shotgun UMETA(DisplayName = "ShotgunWeapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps)const override;
	virtual void OnRep_Owner()override;

	/*
		enable/disable custom depth
	*/
	void EnableCustomDepth(bool bEnable);

protected:
	virtual void BeginPlay() override;
	/*
		WeaponState
	*/
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();

	UFUNCTION()
		virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComponnet,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);

	UFUNCTION()
		virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComponent,
			int32 OtherBodyIndex);

	/*
		TraceEnd with scatter
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Scatter")
		float DistanceToSphere = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Scatter")
		float SphereRadius = 150.f;

	UPROPERTY(EditDefaultsOnly)
		float Damage = 20.f;

	/*
		Server side rewind
	*/
	UPROPERTY(Replicated, EditDefaultsOnly)
		bool bUseServerSideRewind = false;

	UFUNCTION()
		void OnPingTooHigh(bool bPingTooHigh);

	class ABlasterCharacter* BlasterOwnerCharacter = nullptr;
	class ABlasterPlayerController* BlasterOwnerController = nullptr;
private:

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
		USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
		class USphereComponent* AreaSphere;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties")
		class USoundCue* EquippedSound;
	/*
		WeaponState
	*/
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "WeaponProperties")
		EWeaponState WeaponState;
	UFUNCTION()
		void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
		class UWidgetComponent* PickupWidget;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties")
		class UAnimationAsset* FireAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties")
		TSubclassOf<class ACasing> CasingClass;

	//Zoomed FOV when Aiming
	UPROPERTY(EditDefaultsOnly)
		float ZoomedFOV = 30.f;
	UPROPERTY(EditDefaultsOnly)
		float ZoomInterpSpeed = 20.f;

	//automatic fire
	UPROPERTY(EditDefaultsOnly)
		float FireRate = 0.15f;
	UPROPERTY(EditDefaultsOnly)
		bool bIsAutomatic = true;

	//scatter
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Scatter")
		bool bUseScatter = false;

	//ammo
	UPROPERTY(EditDefaultsOnly)
		int32 Ammo = 30;
	UFUNCTION(Client, Reliable)
		void ClientUpdateAmmo(int32 ServerAmmo);
	UFUNCTION(Client, Reliable)
		void ClientAddAmmo(int32 AmmoToAdd);

	/*	client-prediction ammo
		The number if unprocessed server requests for ammo
		Invremented in SpeedAmmo(), decremented in ClientUpdateAmmo()
	*/
	int32 Sequence = 0;

	UPROPERTY(EditDefaultsOnly)
		int32 MagCapacity = 30;

	void SpendRound();

	/*
		Weapon Props
	*/
	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties")
		EWeaponType WeaponType;
	UPROPERTY(EditDefaultsOnly, Category = "WeaponProperties")
		EFireType FireType;

	//default weapon
	bool bIsAutoDestroyWeapon = false;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
		class UTexture2D* CrosshairsCenter;
	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
		UTexture2D* CrosshairsLeft;
	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
		UTexture2D* CrosshairsRight;
	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
		UTexture2D* CrosshairsTop;
	UPROPERTY(EditDefaultsOnly, Category = "Crosshairs")
		UTexture2D* CrosshairsBottom;

	void ShowPickupWidget(bool bShouldShowWidget);
	void SetWeaponState(EWeaponState State);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();

	void SetHUDAmmo();

	/*
		Get Weapon Assets
	*/
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh()const { return WeaponMesh; }
	FORCEINLINE USoundCue* GetEquipSound()const { return EquippedSound; }

	/*
		Get Weapon Properties
	*/
	FORCEINLINE float GetZoomedFOV()const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE float GetFireRate() const { return FireRate; }
	FORCEINLINE bool GetFireMode() const { return bIsAutomatic; }
	FORCEINLINE EWeaponType GetWeaponType()const { return WeaponType; }
	FORCEINLINE int32 GetWeaponAmmo()const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity()const { return MagCapacity; }
	FORCEINLINE bool IsAutoDestroy()const { return bIsAutoDestroyWeapon; }
	FORCEINLINE EFireType GetFireType()const { return FireType; }
	FORCEINLINE bool GetUseScatter()const { return bUseScatter; }
	FORCEINLINE float GetBaseDamage()const { return Damage; }
	void SetIsAutoDestroy(bool ShouldAutoDestroy) { bIsAutoDestroyWeapon = ShouldAutoDestroy; }
	void AddAmmo(int32 AmmoToAdd);
	bool IsEmpty() { return Ammo <= 0; }
	bool IsFull() { return Ammo == MagCapacity; }
	FVector TraceEndWithScatter(const FVector& HitTarget);
};
