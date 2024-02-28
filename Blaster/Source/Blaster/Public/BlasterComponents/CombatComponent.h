// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

	void SetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bAinming);

	UFUNCTION()
		void OnRep_EquippedWeapon();

private:
	class ABlasterCharacter* BlasterCharacter;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
		AWeaponBase* EquippedWeapon;

	UPROPERTY(Replicated)
		bool bIsAiming;



public:

};
