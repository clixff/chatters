// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_Base.h"
#include "../Chatters/Character/Equipment/EquipmentList.h"
#include "EquipmentItemFactory.h"
#include "EquipmentListFactory.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERSEDITORMODULE_API UEquipmentListFactory : public UFactory
{
	GENERATED_BODY()
public:

	UEquipmentListFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
	virtual bool CanCreateNew() const override;
};

class CHATTERSEDITORMODULE_API FEquipmentListAssetActions : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return NSLOCTEXT("ChattersEditorModule", "EquipmentList", "Equipment List Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(127, 255, 255); }
	virtual UClass* GetSupportedClass() const override { return UEquipmentList::StaticClass(); };
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }
	virtual uint32 GetCategories() override { return EquipmentCategory; };
};

