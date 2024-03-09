// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Camera/CameraComponent.h"
#include "Interfaces/CrosshairsInterface.h"
#include "BlasterCharacter.generated.h"

class UInputAction;
class UInputMappingContext;

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

	virtual void OnRep_ReplicatedMovement()override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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
		UInputAction* AimingAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase|Weapon", meta = (AllowPrivateAccess))
		UInputAction* FireAction;

	void OnPressedCrouch();
	void OnPressedAiming();
	void OnReleaseAiming();
	void OnFiredButtonPressed();
	void OnFiredButtonRelease();

	/*
		Combat
	*/
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeaponBase* OverlappingWeapon;

	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeaponBase* LastWeapon);

	UPROPERTY(VisibleAnywhere)
		class UCombatComponent* CombatComponent;

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

	void PlayHitReactMontage();

	/*
		Character Stats
	*/
	class ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY(EditDefaultsOnly, Category = "Character Stats")
		float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Character Stats")
		float Health = 100.f;
	UFUNCTION()
		void OnRep_Health();

	UFUNCTION()
	void ReceiveDamage(
		AActor* DamagedActor,
		float Damage,
		const UDamageType* DamageType,
		class AController* InstigatorController,
		AActor* DamageCauser);

	void UpdateHUDHealth();

public:
	/*
		Combat
	*/
	void SetOverlappingWeapon(AWeaponBase* Weapon);
	bool IsEquippedWeapon()const;
	bool IsAiming()const;
	/*
		Anim
	*/
	FORCEINLINE float GetAO_Yaw()const { return AO_Yaw; }
	FORCEINLINE float GetAO_Picth()const { return AO_Pitch; }

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE bool GetShouldRotateRootBone()const { return bShouldRotateRootBone; }

	void PlayFireMontage(bool bAiming);
	/*
		Combat
	*/
	AWeaponBase* GetEquippedWeapon()const;

	FVector GetHitTarget()const;
	/*
		Camera
	*/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
