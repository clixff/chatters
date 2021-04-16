// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentListFactory.h"

UEquipmentListFactory::UEquipmentListFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->SupportedClass = UEquipmentList::StaticClass();
	this->bCreateNew = true;
	this->bEditAfterNew = true;
}

UObject* UEquipmentListFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UEquipmentList>(InParent, InClass, InName, Flags);
}

bool UEquipmentListFactory::ShouldShowInNewMenu() const
{
	return true;
}

bool UEquipmentListFactory::CanCreateNew() const
{
	return true;
}
