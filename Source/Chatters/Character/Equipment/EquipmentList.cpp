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

bool UEquipmentList::IsTeamEquipmentSetsExists()
{
	return this->TeamEquipmentSets.Blue.Num() && this->TeamEquipmentSets.Red.Num();
}

UEquipmentList* UEquipmentList::GetEquipmentSet(EBotTeam Team)
{
	if (Team != EBotTeam::White && this->IsTeamEquipmentSetsExists())
	{
		TArray<UEquipmentList*>& TeamEquipmentArray = Team == EBotTeam::Blue ? this->TeamEquipmentSets.Blue : this->TeamEquipmentSets.Red;
		auto* TeamEquipmentSet = TeamEquipmentArray[FMath::RandRange(0, TeamEquipmentArray.Num() - 1)];
		if (TeamEquipmentSet)
		{
			return TeamEquipmentSet;
		}
	}
	else if (this->EquipmentSets.Num())
	{
		UEquipmentList* RandomEquipmentSet = this->EquipmentSets[FMath::RandRange(0, this->EquipmentSets.Num() - 1)];

		if (RandomEquipmentSet)
		{
			return RandomEquipmentSet;
		}
	}

	return this;
}

FRandomEquipment UEquipmentList::GetRandomEquipment(EBotTeam Team)
{
	auto* EquipmentSet = this->GetEquipmentSet(Team);

	if (EquipmentSet && EquipmentSet != this)
	{
		return EquipmentSet->GetRandomEquipment();
	}

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

	//int32 NumberOfWeapons = this->Weapons.Num();
	//if (NumberOfWeapons)
	//{
	//	RandomIndex = FMath::RandRange(0, NumberOfWeapons - 1);
	//	Equipment.Weapon = this->Weapons[RandomIndex];
	//}

	int32 NumberOfCostumes = this->Costumes.Num();
	if (NumberOfCostumes)
	{
		RandomIndex = FMath::RandRange(0, NumberOfCostumes - 1);
		Equipment.Costume = this->Costumes[RandomIndex];
	}

	return Equipment;
}

UWeaponItem* UEquipmentList::GetRandomWeapon(TArray<bool>& AvailableWeapons, EBotTeam Team)
{
	auto* EquipmentSet = this->GetEquipmentSet(Team);

	if (EquipmentSet && EquipmentSet != this)
	{
		return EquipmentSet->GetRandomWeapon(AvailableWeapons, Team);
	}

	const int32 WeaponsArrayNum = this->Weapons.Num();
	if (!WeaponsArrayNum)
	{
		return nullptr;
	}

	TArray<UWeaponItem*> AvailableWeaponsRefs;

	for (int32 i = 0; i < AvailableWeapons.Num(); i++)
	{
		bool& bWeaponAvailable = AvailableWeapons[i];

		if (bWeaponAvailable && i < WeaponsArrayNum)
		{
			AvailableWeaponsRefs.Add(this->Weapons[i]);
		}
	}

	if (!AvailableWeaponsRefs.Num())
	{
		AvailableWeaponsRefs = this->Weapons;
	}

	if (AvailableWeaponsRefs.Num() == 1)
	{
		return AvailableWeaponsRefs[0];
	}

	int32 RandomIndex = FMath::RandRange(0, AvailableWeaponsRefs.Num() - 1);

	return AvailableWeaponsRefs[RandomIndex];
}
