// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraEnemy.h"

void AAuraEnemy::HighLight()
{
	bShouldHighLight = true;
}

void AAuraEnemy::UnHighLight()
{
	bShouldHighLight = false;
}
