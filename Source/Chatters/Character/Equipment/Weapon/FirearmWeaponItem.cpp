// Fill out your copyright notice in the Description page of Project Settings.


#include "FirearmWeaponItem.h"

UFirearmWeaponItem::UFirearmWeaponItem()
{
	this->Type = EWeaponType::Firearm;
}

UFirearmWeaponItem::~UFirearmWeaponItem()
{

}

FLinearColor UFirearmWeaponItem::GetRandomProjectileColor()
{
	int32 ColorsNum = this->RandomProjectileColors.Num();
	if (!ColorsNum)
	{
		return FLinearColor(1.0f, 1.0f, 1.0f);
	}

	int32 RandomIndex = FMath::RandRange(0, ColorsNum - 1);

	return this->RandomProjectileColors[RandomIndex];
}
