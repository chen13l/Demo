// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/PickupBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Weapon/WeaponTypes.h"

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

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlappedSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	PickupMesh->MarkRenderStateDirty();
	PickupMesh->SetRenderCustomDepth(true);
}

void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority()) {
		OverlappedSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	}
}

void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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