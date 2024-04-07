// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Weapon/WeaponTypes.h"
#include "BlasterComponents/LagCompensationComponent.h"
#include "PlayerController/BlasterPlayerController.h"

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

		FHitResult HitResult;
		WeaponTraceHit(Start, HitTarget, HitResult);

		UWorld* World = GetWorld();
		if (World)
		{
			if (HitResult.bBlockingHit)
			{
				ABlasterCharacter* HitedCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());
				if (HitedCharacter && InstigatorController)
				{
					if (HasAuthority() && !bUseServerSideRewind) {
						UGameplayStatics::ApplyDamage(
							HitedCharacter,
							Damage,
							InstigatorController,
							GetOwner(),
							UDamageType::StaticClass()
						);
					}
					else if (!HasAuthority() && bUseServerSideRewind) {
						BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(InstigatorPawn) : BlasterOwnerCharacter;
						if (BlasterOwnerCharacter) {
							BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
							if (BlasterOwnerController && BlasterOwnerCharacter->GetLagCompensationComponent() && BlasterOwnerCharacter->IsLocallyControlled()) {
								BlasterOwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(
									HitedCharacter,
									Start,
									HitTarget,
									BlasterOwnerController->GetServerTime() - BlasterOwnerController->GetSingleTripTime(),
									this
								);
							}
						}
					}
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

void AHitScanWeapon::WeaponTraceHit(const FVector& HitStart, const FVector& HitTarget, FHitResult& OutHit)
{
	const FVector End = HitStart + (HitTarget - HitStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World) {
		World->LineTraceSingleByChannel(
			OutHit,
			HitStart,
			End,
			ECollisionChannel::ECC_Visibility
		);

		FVector BeamEnd;
		if (OutHit.bBlockingHit) {
			BeamEnd = OutHit.ImpactPoint;
		}
		if (BeamParticle)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticle,
				HitStart
			);
			if (Beam) {
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
