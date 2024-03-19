// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Weapon/WeaponTypes.h"

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

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalie = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalie * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	// 除于ToEndLoc.Size()防止大于double
	FVector End = FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		End,
		FColor::Cyan,
		true);

	return End;
}