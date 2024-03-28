// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/AmmoPickup.h"
#include "Character/BlasterCharacter.h"
#include "BlasterComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& HitResult)
{
	Super::OnSphereOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, HitResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter) {
		UCombatComponent* CombatComp = BlasterCharacter->GetCombatComponent();
		if (CombatComp) {
			CombatComp->PickupAmmo(WeaponType, AmmoAmount);
		}
	}

	Destroy();
}