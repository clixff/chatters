// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EquipmentItem.generated.h"

USTRUCT(BlueprintType)
struct FMaterialSlots
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
		TArray<UMaterialInterface*> Slots;
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class CHATTERS_API UEquipmentItem : public UObject
{
	GENERATED_BODY()
public:
	UEquipmentItem();
	~UEquipmentItem();

	UPROPERTY(EditDefaultsOnly)
		UStaticMesh* StaticMesh;

	UPROPERTY(EditDefaultsOnly)
		TArray<FMaterialSlots> RandomMaterials;

	UPROPERTY(EditDefaultsOnly, Category = "Transform")
		FTransform Transform;

	TArray<UMaterialInterface*> GetRandomMaterials();

	FTransform GetTransform();
};
