// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	if (InstigatorPawn == nullptr) { return; }
	AController* InstigatorController = Cast<AController>(InstigatorPawn->GetController());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket && InstigatorController) {
		const FVector Start = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh()).GetLocation();
		const FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult HitResult;
		UWorld* World = GetWorld();
		if (World) {
			World->LineTraceSingleByChannel(
				HitResult,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);

			if (HitResult.bBlockingHit) {
				ABlasterCharacter* HitedCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());
				if (HitedCharacter && HasAuthority()) {
					UGameplayStatics::ApplyDamage(
						HitedCharacter,
						Damage,
						InstigatorController,
						GetOwner(),
						UDamageType::StaticClass()
					);
				}

				if (ImpactParticle) {
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticle,
						HitResult.ImpactPoint,
						HitResult.ImpactPoint.Rotation()
					);
				}
			}
		}
	}
}