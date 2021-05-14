// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StaticMesh.h"
#include "HatItem.h"
#include "BeardStyle.h"
#include "Weapon/WeaponItem.h"
#include "CostumeItem.h"
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

	FRandomEquipment GetRandomEquipment();
};
