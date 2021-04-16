// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ChattersEditorModule : ModuleRules
{
	public ChattersEditorModule(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "ContentBrowser", "EditorStyle", "AssetTools", "UnrealEd" });

		PrivateDependencyModuleNames.AddRange(new string[] { "AssetTools" });

        // The path for the source files
        PublicIncludePaths.AddRange(new string[] { "ChattersEditorModule" });
		PrivateIncludePaths.AddRange(new string[] { "ChattersEditorModule" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
