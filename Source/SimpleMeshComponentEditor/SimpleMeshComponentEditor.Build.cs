// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class SimpleMeshComponentEditor : ModuleRules
	{
        public SimpleMeshComponentEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
                    "Core",
                    "CoreUObject",
                    "Slate",
                    "SlateCore",
                    "Engine",
                    "EditorFramework",
                    "UnrealEd",
                    "PropertyEditor",
                    "RenderCore",
                    "RHI",
                    "SimpleMeshComponent",
                    "MeshDescription",
                    "StaticMeshDescription",
                    "AssetTools",
                    "AssetRegistry",
                }
				);
		}
	}
}
