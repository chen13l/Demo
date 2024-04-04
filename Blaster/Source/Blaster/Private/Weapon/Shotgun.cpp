// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

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
		for (FVector_NetQuantize HitTarget : HitTargets) {
			FHitResult HitResult;
			WeaponTraceHit(Start, HitTarget, HitResult);

			ABlasterCharacter* HitedCharacter = Cast<ABlasterCharacter>(HitResult.GetActor());
			if (HitedCharacter) {
				if (HitMap.Contains(HitedCharacter)) {
					++HitMap[HitedCharacter];
				}
				else {
					HitMap.Emplace(HitedCharacter, 1);
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
		for (auto HitPair : HitMap) {
			if (HitPair.Key && HasAuthority() && InstigatorController) {
				UGameplayStatics::ApplyDamage(
					HitPair.Key, //character be hit 
					Damage * HitPair.Value, //multiply damage by numbers of hit
					InstigatorController,
					GetOwner(),
					UDamageType::StaticClass()
				);
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
