// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/CharacterBase.h"
#include "AbilitySystemComponent.h"

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

void ACharacterBase::InitPrimaryAttributes() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(IsValid(ASC));
	check(DefaultPrimaryAttributes);
	const FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	const FGameplayEffectSpecHandle EffectSpecHandle = ASC->MakeOutgoingSpec(
		DefaultPrimaryAttributes, 1.f, EffectContextHandle);
	ASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), GetAbilitySystemComponent());
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
