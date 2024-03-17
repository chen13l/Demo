// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) {
		CollisionSphere->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}

	if (TrailSystem) {
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}

	if (ProjectileLoop && ProjectileLoopComponent)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}

void AProjectileRocket::OnHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalInpulse,
	const FHitResult& HitResult)
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority()) {
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

	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&ThisClass::DestroyTimerFinished,
		DestroyTime
	);

	if (ImpactParticle) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, GetActorTransform());
	}
	if (ImpactSound) {
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (RocketMesh) {
		RocketMesh->SetVisibility(false);
	}
	if (CollisionSphere) {
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController()) {
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying()) {
		ProjectileLoopComponent->Stop();
	}
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroyed();
}

void AProjectileRocket::Destroyed()
{

}