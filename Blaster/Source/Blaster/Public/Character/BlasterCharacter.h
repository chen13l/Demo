// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BlasterCharacter.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps)const override;
	virtual void PostInitializeComponents()override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void Move(const FInputActionValue& Vaule);
	void Look(const FInputActionValue& Value);
	void EquipButtonPressed();

	void AimOffset(float DeltaSeconds);

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
		class UWidgetComponent* OverheadWidget;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase", meta = (AllowPrivateAccess))
		UInputAction* EquipAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MovementBase", meta = (AllowPrivateAccess))
		UInputAction* AimingAction;

	void OnPressedCrouch();
	void OnPressedAiming();
	void OnReleaseAiming();

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeaponBase* OverlappingWeapon;

	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeaponBase* LastWeapon);

	UPROPERTY(VisibleAnywhere)
		class UCombatComponent* CombatComponent;

	UFUNCTION(Server,Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float AO_Pitch;
	FRotator BaseRotation;

public:
	void SetOverlappingWeapon(AWeaponBase* Weapon);
	bool IsEquippedWeapon()const;
	bool IsAiming()const;

	FORCEINLINE float GetAO_Yaw()const { return AO_Yaw; }
	FORCEINLINE float GetAO_Picth()const { return AO_Pitch; }
};
