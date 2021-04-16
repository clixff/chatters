// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentItem.h"

UEquipmentItem::UEquipmentItem()
{
	this->StaticMesh = nullptr;
	this->Location = FVector(0.0f);
	this->Rotation = FRotator(0.0f);
	this->Scale = FVector(1.0f);
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
