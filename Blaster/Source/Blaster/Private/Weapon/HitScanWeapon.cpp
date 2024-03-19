// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	if (InstigatorPawn == nullptr) { return; }
	AController* InstigatorController = Cast<AController>(InstigatorPawn->GetController());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket) {
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		const FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult HitResult;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(
				HitResult,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);
			FVector BeamEnd;
			if (HitResult.bBlockingHit)
			{
				BeamEnd = HitResult.ImpactPoint;

				ABlasterCharacter* HitedCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());
				if (HitedCharacter) 
				{
					UGameplayStatics::ApplyDamage(
						HitedCharacter,
						Damage,
						InstigatorController,
						GetOwner(),
						UDamageType::StaticClass()
					);
				}

				if (ImpactParticle && HasAuthority() && InstigatorController) 
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticle,
						HitResult.ImpactPoint,
						HitResult.ImpactPoint.Rotation()
					);
				}
				if (HitSound) {
					UGameplayStatics::SpawnSoundAtLocation(
						this,
						HitSound,
						HitResult.ImpactPoint
					);
				}
			}
			if (BeamParticle)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticle,
					SocketTransform
				);
				if (Beam) {
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
		if (MuzzleFlash) {
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound) {
			UGameplayStatics::SpawnSoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}