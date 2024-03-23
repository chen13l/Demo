// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/CombatComponent.h"
#include "Weapon/WeaponBase.h"
#include "Character/BlasterCharacter.h"
#include "Character/BlasterAnimInstance.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkkSpeed = 450.f;
	AimWalkSpeed = 300.f;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (BlasterCharacter) {
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkkSpeed;
		if (BlasterCharacter->GetFollowCamera()) {
			DefaultFOV = BlasterCharacter->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (BlasterCharacter && BlasterCharacter->IsLocallyControlled()) {
		FHitResult HitResult;
		TraceUnderCrossHair(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshair(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) { return; }

	if (bIsAiming) {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (BlasterCharacter && BlasterCharacter->GetFollowCamera()) {
		BlasterCharacter->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{

	if (BlasterCharacter == nullptr || BlasterCharacter->Controller == nullptr) { return; }
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController) {
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(BlasterController->GetHUD()) : BlasterHUD;
		if (BlasterHUD) {
			if (EquippedWeapon) {
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairsRight;
			}
			else {
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairBottom = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;
			}
			//speed mapped to [0,1]
			FVector2D WalkSpeedRange(0.f, BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = BlasterCharacter->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (BlasterCharacter->GetCharacterMovement()->IsFalling()) {
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.f, DeltaTime, 2.f);
			}
			else {
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bIsAiming) {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread =
				0.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;

			BlasterHUD->SetHUDPackage(HUDPackage);
		}
	}
}


void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarryAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::OnRep_CarryAmmo()
{
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController) {
		BlasterController->SetHUDCarryAmmo(CarryAmmo);
	}
	bool bShouldJumpToShotgunEnd =
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarryAmmo == 0;
	if (bShouldJumpToShotgunEnd) {
		JumpToSectionEnd();
	}
}

void UCombatComponent::InitializeCarruedAmmo(EWeaponType WeaponType)
{
	if (StartingCarriedAmmo[WeaponType]) {
		CarriedAmmoMap.Emplace(WeaponType, StartingCarriedAmmo[WeaponType]);
	}
	else {
		CarriedAmmoMap.Emplace(WeaponType, 0);
	}
}

void UCombatComponent::EquipWeapon(AWeaponBase* WeaponToEquip)
{
	if (BlasterCharacter == nullptr || WeaponToEquip == nullptr) { return; }
	if (EquippedWeapon != nullptr)
	{
		EquippedWeapon->Dropped();
	}
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = BlasterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket) {
		HandSocket->AttachActor(EquippedWeapon, BlasterCharacter->GetMesh());
	}
	EquippedWeapon->SetOwner(BlasterCharacter);
	EquippedWeapon->SetHUDAmmo();
	InitializeCarruedAmmo(EquippedWeapon->GetWeaponType());
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		CarryAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController) {
		BlasterController->SetHUDCarryAmmo(CarryAmmo);
	}
	if (EquippedWeapon->EquippedSound) {
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->EquippedSound,
			BlasterCharacter->GetActorLocation()
		);
	}

	if (EquippedWeapon->IsEmpty()) {
		Reload();
	}

	BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	BlasterCharacter->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && BlasterCharacter) {

		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = BlasterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket) {
			HandSocket->AttachActor(EquippedWeapon, BlasterCharacter->GetMesh());
		}
		if (EquippedWeapon->EquippedSound) {
			UGameplayStatics::PlaySoundAtLocation(
				this,
				EquippedWeapon->EquippedSound,
				BlasterCharacter->GetActorLocation()
			);
		}
		BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		BlasterCharacter->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::OnFiredButtonPressed(bool bPressed)
{
	bWantFire = bPressed;
	if (bWantFire) {
		Fire();
	}
	else if (!bWantFire && !EquippedWeapon->GetFireMode()) {
		bCanFire = true;
	}
}

void UCombatComponent::Fire()
{
	if (CanFire()) {
		bCanFire = false;
		ServerFire(HitTarget);
		if (EquippedWeapon) {
			CrosshairShootingFactor = 0.75f;
		}
		StartFireTimer();
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) { return false; }
	if (!EquippedWeapon->IsEmpty() &&
		bCanFire &&
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) {
		return true;
	}
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || BlasterCharacter == nullptr) { return; }
	const float FireRate = EquippedWeapon->GetFireRate();
	BlasterCharacter->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&ThisClass::EndFireTimer,
		FireRate
	);
}

void UCombatComponent::EndFireTimer()
{
	if (EquippedWeapon == nullptr) { return; }
	if (bWantFire && EquippedWeapon->GetFireMode()) {
		bCanFire = true;
	}

	if (EquippedWeapon->IsEmpty()) {
		Reload();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) { return; }


	MulticastFire(TraceHitTarget);

}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (BlasterCharacter &&
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) {

		BlasterCharacter->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
		return;
	}
	if (BlasterCharacter && CombatState == ECombatState::ECS_Unoccupied) {
		BlasterCharacter->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::SetAiming(bool bAiming)
{
	/*
	* call RPC on client - owner actor will run on server,
	* 加这一行能使client在收到server信息前先执行动画
	*/
	if (BlasterCharacter == nullptr || EquippedWeapon == nullptr) { return; }
	bIsAiming = bAiming;
	ServerSetAiming(bAiming);
	if (BlasterCharacter) {
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkkSpeed;
	}
	if (BlasterCharacter->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle) {
		BlasterCharacter->ShowScopeWidget(bIsAiming);
	}
}


void UCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;
	if (BlasterCharacter) {
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkkSpeed;
	}
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) { return 0; }
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetWeaponAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		int32 CarriedAmount = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, CarriedAmount);
		return Least;
	}

	return 0;
}

void UCombatComponent::Reload()
{
	if (CarryAmmo > 0 && CombatState != ECombatState::ECS_Reloading) {
		ServerReload();
	}
}

void UCombatComponent::ShotgunShellReload()
{
	if (BlasterCharacter && BlasterCharacter->HasAuthority()) {
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::FinishReloading()
{
	if (BlasterCharacter == nullptr) { return; }
	if (BlasterCharacter->HasAuthority()) {
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bWantFire)
	{
		Fire();
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (BlasterCharacter == nullptr || EquippedWeapon == nullptr) { return; }
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarryAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController) {
		BlasterController->SetHUDCarryAmmo(CarryAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (BlasterCharacter == nullptr || EquippedWeapon == nullptr) { return; }

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarryAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController) {
		BlasterController->SetHUDCarryAmmo(CarryAmmo);
	}
	EquippedWeapon->AddAmmo(-1);
	bCanFire = true;

	if (EquippedWeapon->IsFull() || CarryAmmo == 0) {
		JumpToSectionEnd();
	}
}

void UCombatComponent::JumpToSectionEnd()
{
	//jump to ShotgunEnd Section
	UAnimInstance* AnimInstance = BlasterCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance && BlasterCharacter->GetReloadMontage() || CarryAmmo == 0) {
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (BlasterCharacter == nullptr || EquippedWeapon == nullptr) { return; }

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::HandleReload()
{
	BlasterCharacter->PlayReloadMontage();
}


void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (bWantFire) {
			Fire();
		}
		break;
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_MAX:
		break;
	default:
		break;
	}
}

void UCombatComponent::TraceUnderCrossHair(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D Corsshair(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		Corsshair,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld) {
		FVector Start = CrosshairWorldPosition;

		if (BlasterCharacter) {
			float DistanceToSelf = (BlasterCharacter->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToSelf + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UCrosshairsInterface>()) {
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else {
			HUDPackage.CrosshairColor = FLinearColor::White;
		}

		if (!TraceHitResult.bBlockingHit) {
			TraceHitResult.ImpactPoint = End;
		}
	}
}



