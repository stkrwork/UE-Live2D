#include "Live2DMotionEditorModule.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions_Live2DModelMotion.h"
#include "IAssetTools.h"
#include "Live2DMotionEditor.h"

#define LOCTEXT_NAMESPACE "FLive2DMotionEditorModule"

const FName Live2DMotionEditorAppIdentifier = FName(TEXT("Live2DMotionEditorApp"));


void FLive2DMotionEditorModule::StartupModule()
{
	// Create new extensibility managers for our menu and toolbar
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);
	
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_Live2DModelMotion));	
}

void FLive2DMotionEditorModule::ShutdownModule()
{
	// Reset our existing extensibility managers
	MenuExtensibilityManager.Reset();
	ToolBarExtensibilityManager.Reset();
	

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		// Unregister our custom created assets from the AssetTools
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 i = 0; i < CreatedAssetTypeActions.Num(); ++i)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[i].ToSharedRef());
		}
	}

	CreatedAssetTypeActions.Empty();
}

TSharedRef<ILive2DMotionEditor> FLive2DMotionEditorModule::CreateLive2dMotionEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, ULive2DModelMotion* Live2DModelMotion)
{
	// Initialize and spawn a new Live2D Motion editor with the provided parameters
	TSharedRef<FLive2DMotionEditor> NewLive2DMotionEditor(new FLive2DMotionEditor());
	NewLive2DMotionEditor->InitLive2DMotionEditor(Mode, InitToolkitHost, Live2DModelMotion);
	return NewLive2DMotionEditor;
}

void FLive2DMotionEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_GAME_MODULE(FLive2DMotionEditorModule, Live2DMotionEditor)