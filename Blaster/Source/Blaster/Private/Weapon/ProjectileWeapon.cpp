// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget) {
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket&&World) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//from muzzle flash socket to hit location from TraceUnderCrossHairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		AProjectile* SpawnProjectile = nullptr;
		if (bUseServerSideRewind) {
			if (InstigatorPawn->HasAuthority()) { //server
				if (InstigatorPawn->IsLocallyControlled()) { //server, host - use Replicated projectile
					SpawnProjectile = World->SpawnActor<AProjectile>(ProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParams);
					SpawnProjectile->SetUseServerSideRewind(false);
					SpawnProjectile->SetBaseDamage(Damage);
				}
				else { //server, not Locally Controlled - use non-replicated, no SSR
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnProjectile->SetUseServerSideRewind(false);
				}
			}
			else { // client, using SSR
				if (InstigatorPawn->IsLocallyControlled()) { // client, Locally Controlled - use non-replicated, using SSR
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnProjectile->SetUseServerSideRewind(true);
					SpawnProjectile->SetTraceStart(SocketTransform.GetLocation());
					SpawnProjectile->SetInitialVelocity(SpawnProjectile->GetActorForwardVector() * SpawnProjectile->GetInitialSpeed());
					SpawnProjectile->SetBaseDamage(Damage);
				}
				else { // client,, not Locally Controlled - use non-replicated, no SSR
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnProjectile->SetUseServerSideRewind(false);
				}
			}
		}
		else { // weapon not using SSR
			if (InstigatorPawn->HasAuthority()) { // server, all use replicated 
				SpawnProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnProjectile->SetUseServerSideRewind(false);
				SpawnProjectile->SetBaseDamage(Damage);
			}
		}
	}
}