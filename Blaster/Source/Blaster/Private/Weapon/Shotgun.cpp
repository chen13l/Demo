// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeaponBase::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	if (InstigatorPawn == nullptr) { return; }
	AController* InstigatorController = Cast<AController>(InstigatorPawn->GetController());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket) {
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		//calculate hit characters 
		TMap<ABlasterCharacter*, uint32>HitMap;
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult HitResult;
			WeaponTraceHit(Start, HitTarget, HitResult);

			ABlasterCharacter* HitedCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());
			if (HitedCharacter && HasAuthority() && InstigatorController) {
				if (HitMap.Contains(HitedCharacter)) {
					++HitMap[HitedCharacter];
				}
				else {
					HitMap.Emplace(HitedCharacter, 1);
				}

				if (ImpactParticle) {
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticle,
						HitResult.ImpactPoint,
						FRotator::ZeroRotator,
						true
					);
				}
				if (HitSound) {
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						HitResult.ImpactPoint,
						.5f,
						FMath::RandRange(-.5f, .5f)
					);
				}
			}
		}

		//calculate total damage for each hit character
		for (auto HitPair : HitMap) {
			UGameplayStatics::ApplyDamage(
				HitPair.Key,
				Damage * HitPair.Value,
				InstigatorController,
				GetOwner(),
				UDamageType::StaticClass()
			);
		}
	}
}