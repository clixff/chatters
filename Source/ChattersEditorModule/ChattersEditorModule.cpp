
#include "ChattersEditorModule.h"
#include "EquipmentItemFactory.h"
#include "EquipmentListFactory.h"

#define LOCTEXT_NAMESPACE "ChattersEditorModule"

void FChattersEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	EquipmentCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("EquipmentCategory")), NSLOCTEXT("ChattersEditorModule", "EquipmentCategory", "Equipment"));

	AssetTools.RegisterAssetTypeActions(MakeShareable(new FEquipmentListAssetActions()));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FHatItemAssetActions()));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FBeardStyleAssetActions()));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FMeleeWeaponAssetActions()));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FFirearmWeaponAssetActions()));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FCostumeItemAssetActions()));
}

void FChattersEditorModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
 
IMPLEMENT_GAME_MODULE(FChattersEditorModule, ChattersEditorModule)