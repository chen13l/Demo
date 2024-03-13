// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UENUM(BlueprintType)
enum class EWeaponState :uint8 {
	EWS_Initial UMETA(DisplayName = "Initail State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
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
protected:
	virtual void BeginPlay() override;

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
private:
	class ABlasterCharacter* BlasterOwnerCharacter = nullptr;
	class ABlasterPlayerController* BlasterOwnerController = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
		USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
		class USphereComponent* AreaSphere;

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

	UPROPERTY(EditDefaultsOnly)
		float FireRate = 0.15f;
	UPROPERTY(EditDefaultsOnly)
		bool bIsAutomatic = true;

	//ammo
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_Ammo)
		int32 Ammo = 30;
	UFUNCTION()
		void OnRep_Ammo();
	UPROPERTY(EditDefaultsOnly)
		int32 MagCapacity = 30;

	void SpendRound();

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

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh()const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV()const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE float GetFireRate() const { return FireRate; }
	FORCEINLINE bool GetFireMode() const { return bIsAutomatic; }
};
