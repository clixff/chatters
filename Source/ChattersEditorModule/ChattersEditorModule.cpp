
#include "ChattersEditorModule.h"
#include "EquipmentItemFactory.h"
#include "EquipmentListFactory.h"

#define LOCTEXT_NAMESPACE "ChattersEditorModule"

void FChattersEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	EquipmentCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("EquipmentCategory")), NSLOCTEXT("ChattersEditorModule", "EquipmentCategory", "Equipment Category"));

	AssetTools.RegisterAssetTypeActions(MakeShareable(new FEquipmentListAssetActions()));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FEquipmentItemAssetActions()));
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FHatItemAssetActions()));
}

void FChattersEditorModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
 
IMPLEMENT_GAME_MODULE(FChattersEditorModule, ChattersEditorModule)