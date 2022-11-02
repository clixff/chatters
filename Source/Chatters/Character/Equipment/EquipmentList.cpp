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

UEquipmentList* UEquipmentList::GetEquipmentSet(EBotTeam Team, TSet<FString> AllowedWeaponsList)
{
	if (Team == EBotTeam::Zombie && ZombieSet)
	{
		return ZombieSet;
	}

	if (Team != EBotTeam::White && this->IsTeamEquipmentSetsExists())
	{
		TArray<UEquipmentList*> TeamEquipmentArray = Team == EBotTeam::Blue ? this->TeamEquipmentSets.Blue : this->TeamEquipmentSets.Red;

		TArray<UEquipmentList*> TeamEquipmentArrayCopy;

		for (int32 i = 0; i < TeamEquipmentArray.Num(); i++)
		{
			if (TeamEquipmentArray[i]->IsContainsAllowedWeapons(AllowedWeaponsList))
			{
				TeamEquipmentArrayCopy.Add(TeamEquipmentArray[i]);
			}
		}

		if (!TeamEquipmentArrayCopy.Num())
		{
			return TeamEquipmentArray[0];
		}

		auto* TeamEquipmentSet = TeamEquipmentArrayCopy[FMath::RandRange(0, TeamEquipmentArrayCopy.Num() - 1)];
		if (TeamEquipmentSet)
		{
			return TeamEquipmentSet;
		}
	}
	else if (this->EquipmentSets.Num())
	{
		TArray<UEquipmentList*> EquipmentSetsCopy;

		for (int32 i = 0; i < this->EquipmentSets.Num(); i++)
		{
			if (this->EquipmentSets[i]->IsContainsAllowedWeapons(AllowedWeaponsList))
			{
				EquipmentSetsCopy.Add(this->EquipmentSets[i]);
			}
		}

		UEquipmentList* RandomEquipmentSet = EquipmentSetsCopy[FMath::RandRange(0, EquipmentSetsCopy.Num() - 1)];

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

	int32 NumberOfRobots = Robots.Num();
	if (NumberOfRobots)
	{
		RandomIndex = FMath::RandRange(0, NumberOfRobots - 1);
		Equipment.Robot = Robots[RandomIndex];
	}

	return Equipment;
}

UWeaponItem* UEquipmentList::GetRandomWeapon(TSet<FString>& AvailableWeapons, EBotTeam Team)
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

	for (int32 i = 0; i < this->Weapons.Num(); i++)
	{
		auto Name = this->Weapons[i]->GetName();

		if (AvailableWeapons.Contains(Name))
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

bool UEquipmentList::IsContainsAllowedWeapons(TSet<FString> AllowedWeaponsList)
{
	if (!AllowedWeaponsList.Num())
	{
		return true;
	}

	for (int32 i = 0; i < this->Weapons.Num(); i++)
	{
		if (AllowedWeaponsList.Contains(this->Weapons[i]->GetName()))
		{
			return true;
		}
	}

	return false;
}
