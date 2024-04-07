// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Camera/CameraComponent.h"
#include "Interfaces/CrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "BlasterCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UBoxComponent;
class ULagCompensationComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public ICrosshairsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps)const override;
	virtual void PostInitializeComponents()override;
	virtual void Destroyed() override;
	virtual void OnRep_ReplicatedMovement()override;

	/*
		Server rewind
	*/
	TMap<FName, UBoxComponent*> GetHitCollisionBoxes()const { return HitCollisionBoxes; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent()const { return LagCompensation; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//poll for relevent classes & initialize hud
	virtual void PollInit();

	/*
		Input
	*/
	void Move(const FInputActionValue& Vaule);
	void Look(const FInputActionValue& Value);
	void EquipButtonPressed();
	virtual void Jump() override;
	/*
		Anim
	*/
	void AimOffset(float DeltaTime);
	void CaculateAO_Pitch();
	void SimProxiesTurn();

	void RotateInPlace(float DeltaTime);

	/*
		hit boxes for sserver-side rewind
	*/
	void ResgisterHitBoxes();
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* head;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* pelvis;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* spine_02;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* spine_03;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* upperarm_l;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* upperarm_r;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* lowerarm_l;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* lowerarm_r;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* hand_l;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* hand_r;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* backpack;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* blanket;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* thigh_l;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* thigh_r;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* calf_l;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* calf_r;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* foot_l;
	UPROPERTY(EditDefaultsOnly)
		UBoxComponent* foot_r;

	UPROPERTY()
		TMap<FName, UBoxComponent*>HitCollisionBoxes;

private:
	/*
		Camera
	*/
	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
		UCameraComponent* FollowCamera;

	void HideCharacterWhenTooClose();
	UPROPERTY(EditDefaultsOnly)
		float CameraThreshold = 200.f;

	/*
		Widget
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
		class UWidgetComponent* OverheadWidget;
	/*
		Input
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase", meta = (AllowPrivateAccess))
		UInputMappingContext* IMC_MovementBase;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase", meta = (AllowPrivateAccess))
		UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase", meta = (AllowPrivateAccess))
		UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase", meta = (AllowPrivateAccess))
		UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase", meta = (AllowPrivateAccess))
		UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase|Weapon", meta = (AllowPrivateAccess))
		UInputAction* EquipAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase|Weapon", meta = (AllowPrivateAccess))
		UInputAction* SwapWeaponAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase|Weapon", meta = (AllowPrivateAccess))
		UInputAction* AimingAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase|Weapon", meta = (AllowPrivateAccess))
		UInputAction* FireAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase|Weapon", meta = (AllowPrivateAccess))
		UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase|Weapon", meta = (AllowPrivateAccess))
		UInputAction* ThrowGrenadeAction;


	void OnPressedCrouch();
	void OnPressedAiming();
	void OnReleaseAiming();
	void OnFiredButtonPressed();
	void OnFiredButtonRelease();
	void OnReloadButtonPressed();
	void OnGreandeButtonPressed();
	void OnSwapWeapon();

	UPROPERTY(ReplicatedUsing = OnRep_DisableGameplay)
		bool bDisableGameplay = false;
	UFUNCTION()
		void OnRep_DisableGameplay();
	/*
		Combat
	*/
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeaponBase* OverlappingWeapon;

	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeaponBase* LastWeapon);

	/*
		BlasterComponents
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
		class UCombatComponent* CombatComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
		class UBuffComponent* BuffComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
		 ULagCompensationComponent* LagCompensation;

	UFUNCTION(Server, Reliable)
		void ServerEquipButtonPressed();

	/*
		Anim
	*/
	float AO_Yaw;
	float InterpYaw;
	float AO_Pitch;
	FRotator BaseRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);
	bool bShouldRotateRootBone;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementRep;
	float TurnThreshold = 0.5f;
	float CaculateSpeed();

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		class UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		UAnimMontage* HitReactMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		UAnimMontage* ReloadMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		UAnimMontage* ThrowGrenadeMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Elim")
		UAnimMontage* ElimMontage;

	void PlayHitReactMontage();
	void PlayElimMontage();

	//Elim Bot
	UPROPERTY(EditDefaultsOnly, Category = "Elim")
		class UParticleSystem* ElimBotEffect;
	UPROPERTY(VisibleAnywhere, Category = "Elim")
		class UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
		class USoundCue* ElimBotSound;

	/**
	 * Dissolve Effect
	 */
	UPROPERTY(VisibleAnywhere, Category = "Elim")
		UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditDefaultsOnly, Category = "Elim")
		UCurveFloat* DissolveCurve;

	UFUNCTION()
		void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	//Dynamic instance that can change at runtime
	UPROPERTY(VisibleAnywhere, Category = "Elim")
		UMaterialInstanceDynamic* DynamicDissolveMaterialinstance;
	//Material set on Blueprint, used with Dynamic Material Instance
	UPROPERTY(EditDefaultsOnly, Category = "Elim")
		UMaterialInstance* DissolveMaterialInstance;

	/*
		Character Stats
	*/
	class ABlasterPlayerController* BlasterPlayerController = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Character Stats")
		float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Character Stats")
		float Health = 100.f;
	UFUNCTION()
		void OnRep_Health(float LastHealth);

	UPROPERTY(EditDefaultsOnly, Category = "Character Stats")
		float MaxShield = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Character Stats")
		float Shield = 100.f;
	UFUNCTION()
		void OnRep_Shield(float LastShield);

	UFUNCTION()
		void ReceiveDamage(
			AActor* DamagedActor,
			float Damage,
			const UDamageType* DamageType,
			class AController* InstigatorController,
			AActor* DamageCauser);

	bool bIsElim;

	FTimerHandle ElimTimer;
	UPROPERTY(EditDefaultsOnly)
		float ElimDelay = 3.f;
	void ElimTimerFinished();

	class ABlasterPlayerState* BlasterPlayerState = nullptr;

	/*
		Grenade
	*/
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* AttachGrenade;

public:
	/*
		Combat
	*/
	void SetOverlappingWeapon(AWeaponBase* Weapon);
	bool IsEquippedWeapon()const;
	bool IsAiming()const;
	UFUNCTION(BlueprintImplementableEvent)
		void ShowScopeWidget(bool bAiming);

	AWeaponBase* GetEquippedWeapon()const;

	FVector GetHitTarget()const;

	FORCEINLINE float GetHealth()const { return Health; }
	FORCEINLINE float GetMaxHealth()const { return MaxHealth; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }

	FORCEINLINE float GetShield()const { return Shield; }
	FORCEINLINE float GetMaxShield()const { return MaxShield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }

	void UpdateHUDHealth();
	void UpdateHUDShield();

	FORCEINLINE UCombatComponent* GetCombatComponent()const { return CombatComponent; }
	FORCEINLINE UStaticMeshComponent* GetAttachGrenade()const { return AttachGrenade; }
	FORCEINLINE UBuffComponent* GetBuffComponent()const { return BuffComponent; }
	/*
		Anim
	*/
	FORCEINLINE float GetAO_Yaw()const { return AO_Yaw; }
	FORCEINLINE float GetAO_Picth()const { return AO_Pitch; }

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE bool GetShouldRotateRootBone()const { return bShouldRotateRootBone; }

	FORCEINLINE UAnimMontage* GetReloadMontage()const { return ReloadMontage; }

	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayThrowGrenadeMontage();

	/*
		Camera
	*/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/*
		Character Stats
	*/
	FORCEINLINE bool GetDisableGameplay()const { return bDisableGameplay; }
	void Elim();
	void DropOrDestroyWeapons();
	UFUNCTION(NetMulticast, Reliable)
		void MulticastElim();
	FORCEINLINE bool GetIsElim()const { return bIsElim; }

	bool GetLocallyReload()const;

	ECombatState GetCombatState() const;

	void SetDisableGameplay(bool ShouldDisalbe);
};
