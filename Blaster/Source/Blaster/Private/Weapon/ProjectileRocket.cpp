// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalInpulse,
	const FHitResult& HitResult)
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn) {
		AController* FiringController = FiringPawn->GetController();
		if (FiringController) {
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,// World context object
				Damage,//BaseDamage
				10.f,//Minimum Damage
				GetActorLocation(),//Origin(FVector)
				200.f,//DamageInnerRadius
				500.f,//DamageOuterRadius
				1.f,//DamageFalloff
				UDamageType::StaticClass(),//DamageTypeClass
				TArray<AActor*>(),//Ignore Damage Actors
				this,//Damage Causer
				FiringController//Instigator Controller
			);
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalInpulse, HitResult);
}