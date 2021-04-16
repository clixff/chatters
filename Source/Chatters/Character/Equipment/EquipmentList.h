// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StaticMesh.h"
#include "HatItem.h"
#include "EquipmentList.generated.h"

USTRUCT(BlueprintType)
struct FRandomEquipment
{
	GENERATED_BODY()
public:
	UHatItem* Hat;
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

	FRandomEquipment GetRandomEquipment();
};
