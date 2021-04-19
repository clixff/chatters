// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkeletalMesh.h"
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
		TArray<FMaterialSlots> RandomMaterials;

	TArray<UMaterialInterface*> GetRandomMaterials();
};


UCLASS()
class CHATTERS_API UStaticMeshEquipmentItem : public UEquipmentItem
{
	GENERATED_BODY()
public:
	UStaticMeshEquipmentItem();
	~UStaticMeshEquipmentItem();

	UPROPERTY(EditDefaultsOnly)
		UStaticMesh* StaticMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Transform")
		FTransform Transform;

	FTransform GetTransform();
};

UCLASS()
class CHATTERS_API USkeletalMeshEquipmentItem : public UEquipmentItem
{
	GENERATED_BODY()
public:
	USkeletalMeshEquipmentItem();
	~USkeletalMeshEquipmentItem();

	UPROPERTY(EditDefaultsOnly)
		USkeletalMesh* SkeletalMesh;

};