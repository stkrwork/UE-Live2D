#include "Live2DModelEditorModule.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions_Live2DMocModel.h"
#include "IAssetTools.h"
#include "Live2DModelEditor.h"

#define LOCTEXT_NAMESPACE "FLive2DModelEditorModule"

const FName Live2DModelEditorAppIdentifier = FName(TEXT("Live2DModelEditorApp"));


void FLive2DModelEditorModule::StartupModule()
{
	// Create new extensibility managers for our menu and toolbar
	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);
	
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_Live2DMocModel));	
}

void FLive2DModelEditorModule::ShutdownModule()
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

TSharedRef<ILive2DModelEditor> FLive2DModelEditorModule::CreateLive2dModelEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, ULive2DMocModel* Live2DMocModel)
{
	// Initialize and spawn a new Live2D model editor with the provided parameters
	TSharedRef<FLive2DModelEditor> NewLive2DModelEditor(new FLive2DModelEditor());
	NewLive2DModelEditor->InitLive2DModelEditor(Mode, InitToolkitHost, Live2DMocModel);
	return NewLive2DModelEditor;
}

void FLive2DModelEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_GAME_MODULE(FLive2DModelEditorModule, Live2DModelEditor)