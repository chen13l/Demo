// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/CharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACharacterBase::InitAbilityActorinfo()
{
}

void ACharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GE, float Level) const
{
	check(GE);
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC);
	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	FGameplayEffectSpecHandle GameplayEffectSpecHandle = ASC->MakeOutgoingSpec(GE, Level, EffectContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*GameplayEffectSpecHandle.Data.Get());
}

void ACharacterBase::InitDefaultAttributeByGE() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

void ACharacterBase::AddCharacterAbilities()
{
	if (!HasAuthority()) { return; }
	
	UAuraAbilitySystemComponent* ASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	ASC->AddCharacterAbilities(StartupAbilities);
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
