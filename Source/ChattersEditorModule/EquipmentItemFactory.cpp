// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentItemFactory.h"

UEquipmentItemFactory::UEquipmentItemFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->SupportedClass = UEquipmentItem::StaticClass();
	this->bCreateNew = true;
	this->bEditAfterNew = true;
}

UObject* UEquipmentItemFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UEquipmentItem>(InParent, InClass, InName, Flags);
}

bool UEquipmentItemFactory::ShouldShowInNewMenu() const
{
	return true;
}

bool UEquipmentItemFactory::CanCreateNew() const
{
	return true;
}

UHatItemFactory::UHatItemFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->SupportedClass = UHatItem::StaticClass();
}

UObject* UHatItemFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UHatItem>(InParent, InClass, InName, Flags);
}

UBeardStyleFactory::UBeardStyleFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->SupportedClass = UBeardStyle::StaticClass();
}

UObject* UBeardStyleFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UBeardStyle>(InParent, InClass, InName, Flags);
}

UMeleeWeaponFactory::UMeleeWeaponFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->SupportedClass = UMeleeWeaponItem::StaticClass();
}

UObject* UMeleeWeaponFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UMeleeWeaponItem>(InParent, InClass, InName, Flags);
}

UFirearmWeaponFactory::UFirearmWeaponFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->SupportedClass = UFirearmWeaponItem::StaticClass();
}

UObject* UFirearmWeaponFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UFirearmWeaponItem>(InParent, InClass, InName, Flags);
}

UCostumeItemFactory::UCostumeItemFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->SupportedClass = UCostumeItem::StaticClass();
}


UObject* UCostumeItemFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UCostumeItem>(InParent, InClass, InName, Flags);
}
