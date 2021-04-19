// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_Base.h"
#include "../Chatters/Character/Equipment/EquipmentItem.h"
#include "../Chatters/Character/Equipment/HatItem.h"
#include "../Chatters/Character/Equipment/BeardStyle.h"
#include "../Chatters/Character/Equipment/Weapon/MeleeWeaponItem.h"
#include "../Chatters/Character/Equipment/Weapon/FirearmWeaponItem.h"
#include "EquipmentItemFactory.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERSEDITORMODULE_API UEquipmentItemFactory : public UFactory
{
	GENERATED_BODY()
public:

	UEquipmentItemFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
	virtual bool CanCreateNew() const override;
};

UCLASS()
class CHATTERSEDITORMODULE_API UHatItemFactory : public UEquipmentItemFactory
{
	GENERATED_BODY()
public:

	UHatItemFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

static EAssetTypeCategories::Type EquipmentCategory;

class CHATTERSEDITORMODULE_API FEquipmentItemAssetActions : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return NSLOCTEXT("ChattersEditorModule", "EquipmentItem", "Equipment Item Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(127, 255, 255); }
	virtual UClass* GetSupportedClass() const override { return UEquipmentItem::StaticClass(); };
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }
	virtual uint32 GetCategories() override { return EquipmentCategory; };
};

class CHATTERSEDITORMODULE_API FHatItemAssetActions : public FEquipmentItemAssetActions
{
public:
	virtual FText GetName() const override { return NSLOCTEXT("ChattersEditorModule", "EquipmentHatItem", "Hat Item Asset"); }
	virtual UClass* GetSupportedClass() const override { return UHatItem::StaticClass(); };
};

UCLASS()
class CHATTERSEDITORMODULE_API UBeardStyleFactory : public UEquipmentItemFactory
{
	GENERATED_BODY()
public:

	UBeardStyleFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

class CHATTERSEDITORMODULE_API FBeardStyleAssetActions : public FEquipmentItemAssetActions
{
public:
	virtual FText GetName() const override { return NSLOCTEXT("ChattersEditorModule", "EquipmentBeardStyle", "Beard Style Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(247, 227, 92); }
	virtual UClass* GetSupportedClass() const override { return UBeardStyle::StaticClass(); };
};

UCLASS()
class CHATTERSEDITORMODULE_API UMeleeWeaponFactory : public UEquipmentItemFactory
{
	GENERATED_BODY()
public:

	UMeleeWeaponFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

class CHATTERSEDITORMODULE_API FMeleeWeaponAssetActions : public FEquipmentItemAssetActions
{
public:
	virtual FText GetName() const override { return NSLOCTEXT("ChattersEditorModule", "EquipmentMeleeWeapon", "Melee Weapon Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(172, 255, 127); }
	virtual UClass* GetSupportedClass() const override { return UMeleeWeaponItem::StaticClass(); };
};

UCLASS()
class CHATTERSEDITORMODULE_API UFirearmWeaponFactory : public UEquipmentItemFactory
{
	GENERATED_BODY()
public:

	UFirearmWeaponFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

class CHATTERSEDITORMODULE_API FFirearmWeaponAssetActions : public FEquipmentItemAssetActions
{
public:
	virtual FText GetName() const override { return NSLOCTEXT("ChattersEditorModule", "EquipmentFirearmWeapon", "Firearm Weapon Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(212, 76, 133); }
	virtual UClass* GetSupportedClass() const override { return UFirearmWeaponItem::StaticClass(); };
};