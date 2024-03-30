// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/SpeedPickup.h"
#include "Character/BlasterCharacter.h"
#include "BlasterComponents/BuffComponent.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& HitResult)
{
	Super::OnSphereOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, HitResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter) {
		UBuffComponent* BuffComponent = BlasterCharacter->GetBuffComponent();
		if (BuffComponent) {
			BuffComponent->SpeedBuff(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}

		Destroy();
	}
}