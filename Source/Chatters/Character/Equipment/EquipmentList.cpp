// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentList.h"

UEquipmentList::UEquipmentList()
{
	this->Hats.Add(nullptr);
	this->BeardStyles.Add(nullptr);
	this->Weapons.Add(nullptr);
}


UEquipmentList::~UEquipmentList()
{
}

FRandomEquipment UEquipmentList::GetRandomEquipment()
{
	FRandomEquipment Equipment;

	int32 RandomIndex = 0;

	int32 NumberOfHats = this->Hats.Num();
	if (NumberOfHats)
	{
		RandomIndex = FMath::RandRange(0, NumberOfHats - 1);
		Equipment.Hat = this->Hats[RandomIndex];
	}

	int32 NumberOfBeards = this->BeardStyles.Num();
	int32 NumberOfFaceMaterials = this->AdditionalFaceMaterials.Num();
	int32 NumberOfFaceModifications = NumberOfBeards + NumberOfFaceMaterials;
	if (NumberOfFaceModifications)
	{
		RandomIndex = FMath::RandRange(0, NumberOfFaceModifications - 1);
		if (RandomIndex < NumberOfBeards)
		{
			Equipment.BeardStyle = this->BeardStyles[RandomIndex];
			Equipment.FaceMaterial = nullptr;
		}
		else
		{
			Equipment.FaceMaterial = this->AdditionalFaceMaterials[RandomIndex - NumberOfBeards];
			Equipment.BeardStyle = nullptr;
		}
	}

	int32 NumberOfWeapons = this->Weapons.Num();
	if (NumberOfWeapons)
	{
		RandomIndex = FMath::RandRange(0, NumberOfWeapons - 1);
		Equipment.Weapon = this->Weapons[RandomIndex];
	}

	return Equipment;
}
