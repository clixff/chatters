// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StaticMesh.h"
#include "HatItem.h"
#include "BeardStyle.h"
#include "Weapon/WeaponItem.h"
#include "CostumeItem.h"
#include "../Vehicles/Robots/Robot.h"
#include "EquipmentList.generated.h"

USTRUCT(BlueprintType)
struct FRandomEquipment
{
	GENERATED_BODY()
public:
	UHatItem* Hat = nullptr;
	UBeardStyle* BeardStyle = nullptr;
	UMaterialInterface* FaceMaterial = nullptr;
	UWeaponItem* Weapon = nullptr;
	UCostumeItem* Costume = nullptr;
	TSubclassOf<ARobot> Robot = nullptr;
};

USTRUCT(BlueprintType)
struct FTeamRandomEquipment
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<UEquipmentList*> Blue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<UEquipmentList*> Red;
};

/**
 * 
 */
UCLASS(BlueprintType)
class CHATTERS_API UEquipmentList : public UObject
{
	GENERATED_BODY()
public:
	UEquipmentList();
	~UEquipmentList();
	UPROPERTY(EditDefaultsOnly)
		TArray<UHatItem*> Hats;

	UPROPERTY(EditDefaultsOnly)
		TArray<UBeardStyle*> BeardStyles;

	UPROPERTY(EditDefaultsOnly)
		TArray<UMaterialInterface*> AdditionalFaceMaterials;

	UPROPERTY(EditDefaultsOnly)
		TArray<UWeaponItem*> Weapons;

	UPROPERTY(EditDefaultsOnly)
		TArray<UCostumeItem*> Costumes;

	UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<ARobot>> Robots;

	UPROPERTY(EditDefaultsOnly)
		TArray<UEquipmentList*> EquipmentSets;

	UPROPERTY(EditDefaultsOnly)
		FTeamRandomEquipment TeamEquipmentSets;
	
	bool IsTeamEquipmentSetsExists();

	UEquipmentList* GetEquipmentSet(EBotTeam Team, TSet<FString> AllowedWeaponsList = TSet<FString>());

	FRandomEquipment GetRandomEquipment(EBotTeam Team = EBotTeam::White);

	UWeaponItem* GetRandomWeapon(TSet<FString>& AvailableWeapons, EBotTeam Team = EBotTeam::White);
	
	bool IsContainsAllowedWeapons(TSet<FString> AllowedWeaponsList);
};
