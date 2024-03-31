// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/PickupBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Weapon/WeaponTypes.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

APickupBase::APickupBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	
	OverlappedSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlappedSphere->SetupAttachment(RootComponent);
	OverlappedSphere->SetSphereRadius(150.f);
	OverlappedSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlappedSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlappedSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	OverlappedSphere->AddLocalOffset(FVector(0.f, 0.f, 85.f));

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlappedSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetRelativeScale3D(FVector(5.f, 5.f, 5.f));
	PickupMesh->SetRenderCustomDepth(true);
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComoponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority()) {
		GetWorldTimerManager().SetTimer(
			BindOverlapTimer,
			this,
			&APickupBase::BindOverlapTimerFinished,
			BindOverlapTime
		);
	}
}

void APickupBase::BindOverlapTimerFinished()
{
	OverlappedSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
}

void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PickupMesh) {
		PickupMesh->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}
}

void APickupBase::Destroyed()
{
	Super::Destroyed();

	if (PickupSound) {
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);
	}
	if (PickupEffect) {
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}

void APickupBase::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& HitResult) 
{

}