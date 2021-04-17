// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentItem.h"

UEquipmentItem::UEquipmentItem()
{
	this->StaticMesh = nullptr;
	this->Transform.SetScale3D(FVector(1.0f));
}

UEquipmentItem::~UEquipmentItem()
{

}

TArray<UMaterialInterface*> UEquipmentItem::GetRandomMaterials()
{
	TArray<UMaterialInterface*> Materials;

	if (this->RandomMaterials.Num())
	{
		int32 RandomMaterialIndex = FMath::RandRange(0, this->RandomMaterials.Num() - 1);
		auto& MaterialSlots = this->RandomMaterials[RandomMaterialIndex];
		Materials = MaterialSlots.Slots;
	}

	return Materials;
}

FTransform UEquipmentItem::GetTransform()
{
	return this->Transform;;
}
