// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AProjectileBullet::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalInpulse,
	const FHitResult& HitResult) 
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(OtherActor);
	if (OwnerCharacter) {
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController) {
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalInpulse, HitResult);
}
