// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/LagCompensationComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/WeaponBase.h"

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

	SaveFramePackage();
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (BlasterCharacter == nullptr || !BlasterCharacter->HasAuthority()) { return; }
	if (FrameHistory.Num() <= 1) {
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else {
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		//每一帧需要的时间不一定相同
		while (HistoryLength > MaxRecoedTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}

		if (bShowFramePackage) {
			ShowFramePackage(ThisFrame, FColor::Orange);
		}
	}
}

/*only called on server, GetTimeSeconds() will be Authority time*/
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacter;
	if (BlasterCharacter) {
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = BlasterCharacter;
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
			false, //if true, the boxes will containing(expensive
			4.f
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(
	ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation,
	float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

//overload for shotgun
FShotgunServerSideRewindResult ULagCompensationComponent::ServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations,
	float HitTime)
{
	TArray<FFramePackage> FramesToCkeck;
	for (ABlasterCharacter* HitCharacter : HitCharacters) {
		FramesToCkeck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ConfirmHit(FramesToCkeck, TraceStart, HitLocations);
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;

	if (bReturn) { return FFramePackage(); }

	//Frame history of the HitCharacter
	const TDoubleLinkedList<FFramePackage>& Hitstory = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = Hitstory.GetTail()->GetValue().Time;
	const float NewestHistoryTime = Hitstory.GetHead()->GetValue().Time;
	if (OldestHistoryTime > HitTime) {
		//too far back - too laggy to do SSR
		return FFramePackage();
	}
	//Frame package that we check to verify a hit
	FFramePackage FrameToCheck;
	bool bShouldImterpolate = true;

	if (OldestHistoryTime == HitTime) {
		FrameToCheck = Hitstory.GetTail()->GetValue();
		bShouldImterpolate = false;
	}
	if (NewestHistoryTime <= HitTime) {
		FrameToCheck = Hitstory.GetHead()->GetValue();
		bShouldImterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Hitstory.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = Hitstory.GetHead();
	while (Older->GetValue().Time > HitTime) {
		if (Older->GetNextNode() == nullptr) { break; }
		Older = Older->GetNextNode();
		if (HitTime > Older->GetValue().Time) {
			Younger = Older->GetPrevNode();
		}
	}

	if (Older->GetValue().Time == HitTime) {
		FrameToCheck = Older->GetValue();
		bShouldImterpolate = false;
	}

	if (bShouldImterpolate) {
		//Interpolate between younger and older
		FrameToCheck = InterBetweenFrame(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	FrameToCheck.Character = HitCharacter;

	return FrameToCheck;
}


FFramePackage ULagCompensationComponent::InterBetweenFrame(
	const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame,
	float HitTme)
{
	const float TimeDistance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTme - OlderFrame.Time) / TimeDistance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTme;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo) {
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerPair.Value;

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(
	const FFramePackage& Package,
	ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr)return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);

	EnableCharacterCollision(HitCharacter, ECollisionEnabled::NoCollision);

	//enable collision for the head
	UBoxComponent* HeadBox = HitCharacter->GetHitCollisionBoxes()[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World) {
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECollisionChannel::ECC_Visibility
		);
		//we hit the head, return early
		if (ConfirmHitResult.bBlockingHit) {
			ResetMoveBoxes(HitCharacter, CurrentFrame);
			EnableCharacterCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true,true };
		}
		//didn't hit the head, check the rest of the boxes
		else {
			for (auto& HitBoxPair : HitCharacter->GetHitCollisionBoxes()) {
				if (HitBoxPair.Value != nullptr) {
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECollisionChannel::ECC_Visibility
			);
			if (ConfirmHitResult.bBlockingHit) {
				ResetMoveBoxes(HitCharacter, CurrentFrame);
				EnableCharacterCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true,false };
			}
		}
	}
	ResetMoveBoxes(HitCharacter, CurrentFrame);
	EnableCharacterCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false,false };
}

//overload ConfirmHit() for Shotgun
FShotgunServerSideRewindResult ULagCompensationComponent::ConfirmHit(
	const TArray<FFramePackage>& Packages,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : Packages) {
		if (Frame.Character == nullptr) { return FShotgunServerSideRewindResult(); }
	}
	FShotgunServerSideRewindResult ShotgunResult;

	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : Packages) {
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		MoveBoxes(Frame.Character, Frame);
		EnableCharacterCollision(Frame.Character, ECollisionEnabled::NoCollision);

		CurrentFrames.Add(CurrentFrame);
	}
	for (auto& Frame : Packages) {
		//enable collision for the head first
		UBoxComponent* HeadBox = Frame.Character->GetHitCollisionBoxes()[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	}

	UWorld* World = GetWorld();
	//check for head shots
	for (auto& HitLocation : HitLocations) {
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World) {
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECollisionChannel::ECC_Visibility
			);
			ABlasterCharacter* HitedCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (HitedCharacter) {
				if (ShotgunResult.HeadShots.Contains(HitedCharacter)) {
					++ShotgunResult.HeadShots[HitedCharacter];
				}
				else {
					ShotgunResult.HeadShots.Emplace(HitedCharacter, 1);
				}
			}
		}
	}

	//enable collision for all boxes but headbox
	for (auto& Frame : Packages) {
		for (auto& HitBoxPair : Frame.Character->GetHitCollisionBoxes()) {
			if (HitBoxPair.Value != nullptr) {
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Frame.Character->GetHitCollisionBoxes()[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	//check for body shots
	for (auto& HitLocation : HitLocations) {
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World) {
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECollisionChannel::ECC_Visibility
			);
			ABlasterCharacter* HitedCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (HitedCharacter) {
				if (ShotgunResult.BodyShots.Contains(HitedCharacter)) {
					++ShotgunResult.BodyShots[HitedCharacter];
				}
				else {
					ShotgunResult.BodyShots.Emplace(HitedCharacter, 1);
				}
			}
		}
	}

	for (auto& Frame : CurrentFrames) {
		ResetMoveBoxes(Frame.Character, Frame);
		EnableCharacterCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}

	return ShotgunResult;
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) { return; }

	for (auto& HitBoxPair : HitCharacter->GetHitCollisionBoxes()) {
		if (HitBoxPair.Value != nullptr) {
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) { return; }

	for (auto& HitBoxPair : HitCharacter->GetHitCollisionBoxes()) {
		if (HitBoxPair.Value != nullptr) {
			FBoxInformation PackageBoxInfo = Package.HitBoxInfo[HitBoxPair.Key];
			HitBoxPair.Value->SetWorldLocation(PackageBoxInfo.Location);
			HitBoxPair.Value->SetWorldRotation(PackageBoxInfo.Rotation);
			HitBoxPair.Value->SetBoxExtent(PackageBoxInfo.BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetMoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) { return; }

	for (auto& HitBoxPair : HitCharacter->GetHitCollisionBoxes()) {
		if (HitBoxPair.Value != nullptr) {
			FBoxInformation PackageBoxInfo = Package.HitBoxInfo[HitBoxPair.Key];
			HitBoxPair.Value->SetWorldLocation(PackageBoxInfo.Location);
			HitBoxPair.Value->SetWorldRotation(PackageBoxInfo.Rotation);
			HitBoxPair.Value->SetBoxExtent(PackageBoxInfo.BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnable)
{
	if (HitCharacter && HitCharacter->GetMesh()) {
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnable);
	}
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(
	ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation,
	float HitTime,
	AWeaponBase* DamagerCauser)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (BlasterCharacter && HitCharacter && DamagerCauser && Confirm.bHitCinfirm) {
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			DamagerCauser->GetBaseDamage(),
			BlasterCharacter->Controller,
			DamagerCauser,
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(
	const TArray<ABlasterCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations,
	float HitTime)
{
	FShotgunServerSideRewindResult Confirm = ServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto& HitCharacter : HitCharacters) {
		if (HitCharacter == nullptr || BlasterCharacter->GetEquippedWeapon() == nullptr) { continue; }
		float TotalDamage = 0.f;
		if (Confirm.HeadShots.Contains(HitCharacter)) {
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * BlasterCharacter->GetEquippedWeapon()->GetBaseDamage();
			TotalDamage += HeadShotDamage;
		}
		if (Confirm.BodyShots.Contains(HitCharacter)) {
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * BlasterCharacter->GetEquippedWeapon()->GetBaseDamage();
			TotalDamage += BodyShotDamage;
		}

		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			BlasterCharacter->Controller,
			BlasterCharacter->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}
