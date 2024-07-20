// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
	AutoRun();
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) { return; }

	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(
			ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(
			LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoAcceptanceRadius) { bAutoRunning = false; }
	}
}

void AAuraPlayerController::CursorTrace()
{
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) { return; }

	LastActor = CurrentActor;
	CurrentActor = Cast<IEnemyInterface>(CursorHit.GetActor());
	if (CurrentActor) { bTargeting = true; }
	else { bTargeting = false; }
	if (LastActor != CurrentActor)
	{
		if (LastActor) { LastActor->UnHighLight(); }
		if (CurrentActor) { CurrentActor->HighLight(); }
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer());
	if (Subsystem) { Subsystem->AddMappingContext(AuraContext, 0); }

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputData;
	InputData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputData.SetHideCursorDuringCapture(false);
	SetInputMode(InputData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);

	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed,
	                                       &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator ControllerRotation = GetControlRotation();
	const FRotator YawRotation(0.f, ControllerRotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		bTargeting = CurrentActor ? true : false;
		bAutoRunning = false;
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetAuraASC())
		{
			GetAuraASC()->AbilityInputTagReleased(InputTag);
		}
		return;
	}

	if (bTargeting)
	{
		if (GetAuraASC()) { GetAuraASC()->AbilityInputTagReleased(InputTag); }
	}
	else
	{
		APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(
				this, ControlledPawn->GetActorLocation(),
				CachedDestination);
			if (NavPath)
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}
				CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
				bAutoRunning = true;
			}
		}
		bTargeting = false;
		FollowTime = 0.f;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetAuraASC()) { GetAuraASC()->AbilityInputTagHeld(InputTag); }
		return;
	}

	if (bTargeting)
	{
		if (GetAuraASC()) { GetAuraASC()->AbilityInputTagHeld(InputTag); }
	}
	else
	{
		if (CursorHit.bBlockingHit)
		{
			FollowTime += GetWorld()->GetDeltaSeconds();
			if (CursorHit.bBlockingHit) { CachedDestination = CursorHit.ImpactPoint; }

			if (APawn* ControlledPawn = GetPawn())
			{
				const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
				ControlledPawn->AddMovementInput(WorldDirection);
			}
		}
	}
}


UAuraAbilitySystemComponent* AAuraPlayerController::GetAuraASC()
{
	if (AuraASC == nullptr)
	{
		AuraASC = Cast<UAuraAbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()));
	}
	return AuraASC;
}
