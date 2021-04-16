// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentList.h"

UEquipmentList::UEquipmentList()
{
	this->Hats.Add(nullptr);
}


UEquipmentList::~UEquipmentList()
{
}

FRandomEquipment UEquipmentList::GetRandomEquipment()
{
	FRandomEquipment Equipment;

	int32 NumberOfHats = this->Hats.Num();
	int32 RandHatIndex = FMath::RandRange(0, NumberOfHats - 1);
	Equipment.Hat = this->Hats[RandHatIndex];

	return Equipment;
}
