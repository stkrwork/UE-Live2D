#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IAssetTypeActions;
class IAssetTools;
class ILive2DModelEditor;
class ULive2DMocModel;


extern const FName Live2DModelEditorAppIdentifier;

class FLive2DModelEditorModule : public IModuleInterface, public IHasMenuExtensibility, public IHasToolBarExtensibility
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /** Gets the extensibility managers for outside entities to extend custom asset editor's menus and toolbars */
    virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override { return MenuExtensibilityManager; }
    virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override { return ToolBarExtensibilityManager; }
    
    virtual TSharedRef<ILive2DModelEditor> CreateLive2dModelEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, ULive2DMocModel* Live2DMocModel);

private:
    void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);

    TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;
    
    TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
    TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;
};
