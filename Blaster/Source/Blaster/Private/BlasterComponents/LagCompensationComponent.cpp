// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/LagCompensationComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

/*only called on server, GetTimeSeconds() will be Authority time*/
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacter;
	if (BlasterCharacter) {
		Package.Time = GetWorld()->GetTimeSeconds();
		for (auto& BoxPair : BlasterCharacter->GetHitCollisionBoxes()) {
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			Package.HitBoxInfo.Add(FName(BoxPair.Key), BoxInformation);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& Boxinfo : Package.HitBoxInfo) {
		DrawDebugBox(
			GetWorld(),
			Boxinfo.Value.Location,
			Boxinfo.Value.BoxExtent,
			FQuat(Boxinfo.Value.Rotation),
			Color,
			true //if false, the boxes will containing
		);
	}
}