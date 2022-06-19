#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class ULive2DModelMotion;
class IAssetTypeActions;
class IAssetTools;
class ILive2DMotionEditor;


extern const FName Live2DMotionEditorAppIdentifier;

class FLive2DMotionEditorModule : public IModuleInterface, public IHasMenuExtensibility, public IHasToolBarExtensibility
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /** Gets the extensibility managers for outside entities to extend custom asset editor's menus and toolbars */
    virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override { return MenuExtensibilityManager; }
    virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override { return ToolBarExtensibilityManager; }
    
    virtual TSharedRef<ILive2DMotionEditor> CreateLive2dMotionEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost,  ULive2DModelMotion* Live2DModelMotion);

private:
    void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);

    TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;
    
    TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
    TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;
};
