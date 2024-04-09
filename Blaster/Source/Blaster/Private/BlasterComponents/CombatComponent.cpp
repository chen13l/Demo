// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/CombatComponent.h"
#include "Weapon/WeaponBase.h"
#include "Weapon/Projectile.h"
#include "Character/BlasterCharacter.h"
#include "Character/BlasterAnimInstance.h"
#include "GameMode/BlasterGameMode.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Weapon/Shotgun.h"

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
		SpawnDefautlWeapon();
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

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarryAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
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

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && BlasterCharacter) {

		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		AttachActorToRightHand(EquippedWeapon);

		PlayEquipWeaponSound(EquippedWeapon);

		BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		BlasterCharacter->bUseControllerRotationYaw = true;

		EquippedWeapon->EnableCustomDepth(false);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && BlasterCharacter) {
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);

		AttachActorToBackpack(SecondaryWeapon);

		PlayEquipWeaponSound(SecondaryWeapon);
	}
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
		if (BlasterCharacter && !BlasterCharacter->IsLocallyControlled()) {
			HandleReload();
		};
		break;
	case ECombatState::ECS_MAX:
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (BlasterCharacter && !BlasterCharacter->IsLocallyControlled()) {
			BlasterCharacter->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachGrenade(true);
		}
		break;
	case ECombatState::ECS_SwappingWeapon:
		if (BlasterCharacter && BlasterCharacter->IsLocallyControlled()) {
			BlasterCharacter->PlaySwapWeaponMontage();
		}
	default:
		break;
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::OnRep_Aiming()
{
	if (BlasterCharacter && BlasterCharacter->IsLocallyControlled()) {
		bIsAiming = bAimButtomPressed;
	}
}

void UCombatComponent::SpawnDefautlWeapon()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (BlasterGameMode && World && !BlasterCharacter->GetIsElim() && DefaultWeaponClass)
	{
		AWeaponBase* StartingWeapon = World->SpawnActor<AWeaponBase>(DefaultWeaponClass);
		StartingWeapon->SetIsAutoDestroy(true);
		EquipWeapon(StartingWeapon);
		InitializeStartingHUDAmmo();
	}
}

void UCombatComponent::EquipWeapon(AWeaponBase* WeaponToEquip)
{
	if (BlasterCharacter == nullptr || WeaponToEquip == nullptr) { return; }
	if (CombatState != ECombatState::ECS_Unoccupied) { return; }
	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr) {
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else {
		EquipPrimaryWeapon(WeaponToEquip);
	}

	BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	BlasterCharacter->bUseControllerRotationYaw = true;
}

void UCombatComponent::EquipPrimaryWeapon(AWeaponBase* WeaponToEquip)
{
	if (WeaponToEquip == nullptr)return;
	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachActorToRightHand(WeaponToEquip);

	EquippedWeapon->SetOwner(BlasterCharacter);
	EquippedWeapon->SetHUDAmmo();
	InitializeCarruedAmmo(EquippedWeapon->GetWeaponType());

	UpdateCarriedAmmo();

	PlayEquipWeaponSound(WeaponToEquip);

	AutoReloadWeapon();

}

void UCombatComponent::EquipSecondaryWeapon(AWeaponBase* WeaponToEquip)
{
	if (WeaponToEquip == nullptr)return;

	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);

	AttachActorToBackpack(WeaponToEquip);

	SecondaryWeapon->SetOwner(BlasterCharacter);
	PlayEquipWeaponSound(SecondaryWeapon);
}

void UCombatComponent::SwapWeapons()
{
	if (!CanSwapWeapon() || CombatState != ECombatState::ECS_Unoccupied || BlasterCharacter == nullptr)return;

	BlasterCharacter->PlaySwapWeaponMontage();
	BlasterCharacter->bFinishSawpping = false;
	CombatState = ECombatState::ECS_SwappingWeapon;

	if (SecondaryWeapon) {
		SecondaryWeapon->EnableCustomDepth(false);
	}
}

void UCombatComponent::FinishSawp()
{
	if (BlasterCharacter && BlasterCharacter->HasAuthority()) {
		CombatState = ECombatState::ECS_Unoccupied;
	}
	if (BlasterCharacter) {
		BlasterCharacter->bFinishSawpping = true;
	}
	if (SecondaryWeapon) {
		SecondaryWeapon->EnableCustomDepth(true);
	}
}

void UCombatComponent::FinishSawpAttachWeapon()
{
	AWeaponBase* TemWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TemWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);

}

void UCombatComponent::PlayEquipWeaponSound(AWeaponBase* WeaponToEquip)
{
	if (BlasterCharacter && WeaponToEquip && WeaponToEquip->GetEquipSound()) {
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->GetEquipSound(),
			BlasterCharacter->GetActorLocation()
		);
	}
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon != nullptr)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::OnFiredButtonPressed(bool bPressed)
{
	bWantFire = bPressed;
	if (bWantFire) {
		Fire();
	}
	if (!bWantFire) {
		bCanFire = true;
	}
}

void UCombatComponent::Fire()
{
	if (CanFire()) {
		bCanFire = false;
		if (EquippedWeapon) {
			CrosshairShootingFactor = 0.75f;

			switch (EquippedWeapon->GetFireType())
			{
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
			default:
				break;
			}
		}
		StartFireTimer();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) { return; }

	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (BlasterCharacter && BlasterCharacter->IsLocallyControlled()) { return; }

	LocalFire(TraceHitTarget);
}

void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon) {
		HitTarget = EquippedWeapon->GetUseScatter() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		LocalFire(HitTarget);
		ServerFire(HitTarget);
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon) {
		HitTarget = EquippedWeapon->GetUseScatter() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		LocalFire(HitTarget);
		ServerFire(HitTarget);
	}
}

void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun) {
		TArray<FVector_NetQuantize> HiTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HiTargets);
		LocalShotgunFire(HiTargets);
		ServerShotgunFire(HiTargets);
	}
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray< FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const  TArray< FVector_NetQuantize>& TraceHitTargets)
{
	if (BlasterCharacter && BlasterCharacter->IsLocallyControlled()) { return; }

	LocalShotgunFire(TraceHitTargets);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
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

void UCombatComponent::LocalShotgunFire(const  TArray< FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);

	if (Shotgun == nullptr || BlasterCharacter == nullptr) { return; }

	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied) {
		BlasterCharacter->PlayFireMontage(bIsAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) { return false; }
	if (bLocallyReloading) {
		if (!EquippedWeapon->IsEmpty() &&
			bCanFire &&
			CombatState == ECombatState::ECS_Reloading &&
			EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) {
			bLocallyReloading = false;
			return true;
		}
		return false;
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

	AutoReloadWeapon();
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

//SetAimging is called locally
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
	if (BlasterCharacter->IsLocallyControlled()) {
		bAimButtomPressed = true;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;
	if (BlasterCharacter) {
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkkSpeed;
	}
}

void UCombatComponent::AutoReloadWeapon()
{
	if (CarryAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && EquippedWeapon->IsEmpty()) {
		Reload();
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
	if (CarryAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading) {
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
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
	bLocallyReloading = false;
	if (BlasterCharacter->HasAuthority()) {
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bWantFire)
	{
		Fire();
	}
}
void UCombatComponent::ServerReload_Implementation()
{
	if (BlasterCharacter == nullptr || EquippedWeapon == nullptr) { return; }

	CombatState = ECombatState::ECS_Reloading;
	if (!BlasterCharacter->IsLocallyControlled()) {
		HandleReload();
	};
}

void UCombatComponent::HandleReload()
{
	BlasterCharacter->PlayReloadMontage();
}

void UCombatComponent::InitializeCarruedAmmo(EWeaponType WeaponType)
{
	if (StartingCarriedAmmo.Contains(WeaponType)) {
		if (CarriedAmmoMap.Contains(WeaponType)) {
			CarriedAmmoMap[WeaponType] += StartingCarriedAmmo[WeaponType];
		}
		else {
			CarriedAmmoMap.Emplace(WeaponType, StartingCarriedAmmo[WeaponType]);
		}
	}
	else {
		CarriedAmmoMap.Emplace(WeaponType, 0);
	}
}

void UCombatComponent::InitializeStartingHUDAmmo()
{
	if (EquippedWeapon == nullptr) { return; }
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController) {
		UpdateCarriedAmmo();
		BlasterController->SetHUDAmmo(EquippedWeapon->GetWeaponAmmo());
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType)) {
		CarriedAmmoMap[WeaponType] += AmmoAmount;

		UpdateCarriedAmmo();
	}

	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType) {
		Reload();
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) { return; }
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		CarryAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController) {
		BlasterController->SetHUDCarryAmmo(CarryAmmo);
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
	EquippedWeapon->AddAmmo(ReloadAmount);
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

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) { return; }
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) { return; }
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (BlasterCharacter) {
		BlasterCharacter->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachGrenade(true);
	}
	if (BlasterCharacter && !BlasterCharacter->HasAuthority()) {
		ServerThrowGrenade();
	}
	if (BlasterCharacter && BlasterCharacter->HasAuthority()) {
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) { return; }
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (BlasterCharacter) {
		BlasterCharacter->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	BlasterController = BlasterController == nullptr ? Cast<ABlasterPlayerController>(BlasterCharacter->Controller) : BlasterController;
	if (BlasterController) {
		BlasterController->SetHUDGrenade(Grenades);
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::ShowAttachGrenade(bool ShouldShowGrenade)
{
	if (BlasterCharacter && BlasterCharacter->GetAttachGrenade()) {
		BlasterCharacter->GetAttachGrenade()->SetVisibility(ShouldShowGrenade);
	}
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachGrenade(false);

	if (BlasterCharacter && BlasterCharacter->IsLocallyControlled()) {
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (BlasterCharacter &&
		BlasterCharacter->HasAuthority() &&
		GrenadeClass &&
		BlasterCharacter->GetAttachGrenade())
	{
		const  FVector StartLocation = BlasterCharacter->GetAttachGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = BlasterCharacter;
		SpawnParams.Instigator = BlasterCharacter;
		UWorld* World = GetWorld();
		if (World) {
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartLocation,
				ToTarget.Rotation(),
				SpawnParams
				);
		}
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* AttachActor)
{
	if (BlasterCharacter == nullptr || BlasterCharacter->GetMesh() == nullptr || AttachActor == nullptr) { return; }
	const USkeletalMeshSocket* HandSocket = BlasterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket) {
		HandSocket->AttachActor(AttachActor, BlasterCharacter->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* AttachActor)
{
	if (BlasterCharacter == nullptr || BlasterCharacter->GetMesh() == nullptr || AttachActor == nullptr || EquippedWeapon == nullptr) { return; }
	bool bUsePistolScoket =
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;

	FName SocketName = bUsePistolScoket ? FName("PistolSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = BlasterCharacter->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket) {
		HandSocket->AttachActor(AttachActor, BlasterCharacter->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* AttachActor)
{
	if (BlasterCharacter == nullptr || BlasterCharacter->GetMesh() == nullptr || AttachActor == nullptr)return;
	const USkeletalMeshSocket* BackpackSocket = BlasterCharacter->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket) {
		BackpackSocket->AttachActor(AttachActor, BlasterCharacter->GetMesh());
	}
}