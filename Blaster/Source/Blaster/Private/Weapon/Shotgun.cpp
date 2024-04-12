// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerController/BlasterPlayerController.h"
#include "BlasterComponents/LagCompensationComponent.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeaponBase::Fire(FVector());

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	if (InstigatorPawn == nullptr) { return; }
	AController* InstigatorController = Cast<AController>(InstigatorPawn->GetController());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket) {
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		//calculate hit characters 
		TMap<ABlasterCharacter*, uint32>HitMap;
		TArray<ABlasterCharacter*>HitCharacters;
		TMap<ABlasterCharacter*, uint32>HeadShotsMap;

		for (FVector_NetQuantize HitTarget : HitTargets) {
			FHitResult HitResult;
			WeaponTraceHit(Start, HitTarget, HitResult);

			ABlasterCharacter* HitedCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());
			if (HitedCharacter) {
				const bool bHeadShot = HitResult.BoneName.ToString() == FString("head");

				if (bHeadShot) {
					if (HeadShotsMap.Contains(HitedCharacter)) { ++HeadShotsMap[HitedCharacter]; }
					else { HeadShotsMap.Emplace(HitedCharacter, 1); }
				}
				else {
					if (HitMap.Contains(HitedCharacter)) { ++HitMap[HitedCharacter]; }
					else { HitMap.Emplace(HitedCharacter, 1); }
				}
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
		//calculate total damage for each hit character
		TMap<ABlasterCharacter*, float>DamageMap;//maps character hit to total damage
		for (auto HitPair : HitMap) {
			if (HitPair.Key) {
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
				HitCharacters.AddUnique(HitPair.Key);
			}
		}
		for (auto HeadShotPair : HeadShotsMap) {
			if (HeadShotPair.Key) {
				if (HeadShotsMap.Contains(HeadShotPair.Key)) { HeadShotsMap[HeadShotPair.Key] += HeadShotPair.Value * HeadShotDamage; }
				else { HeadShotsMap.Emplace(HeadShotPair.Key, HeadShotPair.Value * HeadShotDamage); }

				HitCharacters.AddUnique(HeadShotPair.Key);
			}
		}

		for (auto DamagePair : DamageMap) {
			if (DamagePair.Key && InstigatorController) {
				bool bCauseAuthDamage = !bUseServerSideRewind || InstigatorPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage) {
					UGameplayStatics::ApplyDamage(
						DamagePair.Key, //character be hit 
						DamagePair.Value, //Damage calculate in the two for loops above
						InstigatorController,
						GetOwner(),
						UDamageType::StaticClass()
					);
				}
			}
		}

		if (!HasAuthority() && bUseServerSideRewind) {
			BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(InstigatorPawn) : BlasterOwnerCharacter;
			if (BlasterOwnerCharacter) {
				BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
				if (BlasterOwnerController && BlasterOwnerCharacter->GetLagCompensationComponent() && BlasterOwnerCharacter->IsLocallyControlled()) {
					BlasterOwnerCharacter->GetLagCompensationComponent()->ShotgunServerScoreRequest(
						HitCharacters,
						Start,
						HitTargets,
						BlasterOwnerController->GetServerTime() - BlasterOwnerController->GetSingleTripTime()
					);
				}
			}
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket == nullptr) { return; }

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalie = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalie * DistanceToSphere;

	for (uint32 i = 0; i < NumberOfPellets; ++i) {
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;

		// 除于ToEndLoc.Size()防止大于double
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}
