// Copyright Epic Games, Inc. All Rights Reserved.

#include "Live2D.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Live2DCubismCore.h"
#include "Live2DLogCategory.h"

#define LOCTEXT_NAMESPACE "FLive2DModule"

namespace
{
	void PrintLive2DLog(const char* LogMessage)
	{
		UE_LOG(LogLive2D, Log, TEXT("%s"), LogMessage);
	}
}

void FLive2DModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("UELive2D")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/Live2DLibrary/Win64/Live2DCubismCore.dll"));
#elif PLATFORM_MAC
    LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/Live2DLibrary/Mac/Release/libExampleLibrary.dylib"));
#elif PLATFORM_LINUX
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/Live2DLibrary/Linux/x86_64-unknown-linux-gnu/libExampleLibrary.so"));
#endif // PLATFORM_WINDOWS

	Live2DLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (Live2DLibraryHandle)
	{
		union
		{
			csmVersion Version;
			uint8 VersionByteArray[4];
			uint16 VersionShortArray[2];
		};
		// Call the test function in the third party library that opens a message box
		Version = csmGetVersion();
		UE_LOG(LogLive2D, Warning, TEXT("Live2D Cubism Version: %hhu.%hhu.%hu"), VersionByteArray[3], VersionByteArray[2], VersionShortArray[0]);
		csmSetLogFunction(PrintLive2DLog);
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load example third party library"));
	}

	AddShaderSourceDirectoryMapping(TEXT("/Plugin/UELive2D"), FPaths::Combine(BaseDir, TEXT("Shaders")));
}

void FLive2DModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(Live2DLibraryHandle);
	Live2DLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLive2DModule, Live2D)
