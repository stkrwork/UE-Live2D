// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using System.Reflection;
using UnrealBuildTool;

public class Live2DLibrary : ModuleRules
{
	private void CopyToBinaries(string Filepath, ReadOnlyTargetRules Target)
	{
		string binariesDir = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "Live2DLibrary", Target.Platform.ToString());
		string filename = Path.GetFileName(Filepath);

		if (!Directory.Exists(binariesDir))
			Directory.CreateDirectory(binariesDir);

		if (!File.Exists(Path.Combine(binariesDir, filename)))
			File.Copy(Filepath, Path.Combine(binariesDir, filename), true);
	}
	
	public Live2DLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Core", "include"));
		PublicDefinitions.Add("CSM_CORE_WIN32_DLL=0");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Core", "dll", "windows", "x86_64", "Live2DCubismCore.lib"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("Live2DCubismCore.dll");

			// Ensure that the DLL is staged along with the executable
			CopyToBinaries(Path.Combine(ModuleDirectory, "Core", "dll", "windows", "x86_64", "Live2DCubismCore.dll"), Target);
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/Live2DLibrary/Win64/Live2DCubismCore.dll");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libExampleLibrary.dylib"));
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/Live2DLibrary/Mac/Release/libExampleLibrary.dylib");
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			string ExampleSoPath = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "Live2DLibrary", "Linux", "x86_64-unknown-linux-gnu", "libExampleLibrary.so");
			PublicAdditionalLibraries.Add(ExampleSoPath);
			PublicDelayLoadDLLs.Add(ExampleSoPath);
			RuntimeDependencies.Add(ExampleSoPath);
		}
	}
}
