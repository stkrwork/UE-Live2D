#pragma once

#include "CoreMinimal.h"
#include "ILive2DModelEditor.h"
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Editor/PropertyEditor/Public/PropertyEditorDelegates.h"


class IDetailsView;
class SDockableTab;
class UMyCustomAsset;

class LIVE2DMODELEDITOR_API FLive2DModelEditor : public ILive2DModelEditor
{
public:
	// This function creates tab spawners on editor initialization
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;

	// This function unregisters tab spawners on editor initialization
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;

	// This method decides how the custom asset editor will be initialized
	void InitLive2DModelEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, ULive2DMocModel* InLive2DMocModel);

	/** Destructor */
	virtual ~FLive2DModelEditor() override;

	/** Begin IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual bool IsPrimaryEditor() const override { return true; }
	virtual void OnClose() override;
	/** End IToolkit interface */


	/** Begin ILive2DModelEditor initerface */
	virtual ULive2DMocModel* GetLive2DMocModel() const override;
	virtual void SetLive2DMocModel(ULive2DMocModel* InLive2DMocModel) override;
	/** End ILive2DModelEditor initerface */

private:
	/** Create the properties tab and its content */
	TSharedRef<SDockTab> SpawnPropertiesTab(const FSpawnTabArgs& Args);
	/** Create the properties tab and its content */
	TSharedRef<SDockTab> SpawnLive2DModelPreviewTab(const FSpawnTabArgs& Args);

	/** Dockable tab for properties */
	TSharedPtr< SDockableTab > PropertiesTab;

	/** Details view */
	TSharedPtr<class IDetailsView> DetailsView;

	/** Details view */
	TSharedPtr<class SImage> Live2DModelPreview;

	/**	The toolkit name */
	static const FName ToolkitFName;

	/**	The tab ids for all the tabs used */
	static const FName PropertiesTabId;

	/**	The tab ids for all the tabs used */
	static const FName Live2DModelPreviewTabId;

	/** The Custom Asset open within this editor */
	ULive2DMocModel* Live2DMocModel = nullptr;
	
	
};
