// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Chatters : ModuleRules
{
	public Chatters(ReadOnlyTargetRules Target) : base(Target)
	{
		bEnableExceptions = true;

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "PhysicsCore", "NavigationSystem", "SlateCore", "Slate", "UMG", "ApexDestruction", "Niagara" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PublicDefinitions.Add("ASIO_STANDALONE");
		PublicDefinitions.Add("_WEBSOCKETPP_CPP11_INTERNAL_");
		PublicDefinitions.Add("_WEBSOCKETPP_CPP11_FUNCTIONAL_");

		PrivateIncludePaths.Add("..\\ThirdParty\\socket.io-client-cpp\\lib\\asio\\asio\\include");
		PrivateIncludePaths.Add("..\\ThirdParty\\socket.io-client-cpp\\lib\\websocketpp");
		PrivateIncludePaths.Add("..\\ThirdParty\\socket.io-client-cpp\\lib\\rapidjson\\include");

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
