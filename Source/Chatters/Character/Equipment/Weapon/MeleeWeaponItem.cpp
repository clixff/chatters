// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponItem.h"

UMeleeWeaponItem::UMeleeWeaponItem()
{
	this->Type = EWeaponType::Melee;
}

UMeleeWeaponItem::~UMeleeWeaponItem()
{

}

FMeleeAnimation UMeleeWeaponItem::GetRandomHitAnimation()
{
	FMeleeAnimation Anim;

	if (HitAnimations.Num())
	{
		Anim = HitAnimations[FMath::RandRange(0, HitAnimations.Num() - 1)];
	}

	return Anim;
}
