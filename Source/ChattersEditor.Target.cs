// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ChattersEditorTarget : TargetRules
{
	public ChattersEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		bCompileChaos = true;
		bUseChaos = true;
		ExtraModuleNames.AddRange( new string[] { "Chatters", "ChattersEditorModule" } );
	}
}
