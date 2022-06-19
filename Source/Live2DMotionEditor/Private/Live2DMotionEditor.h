#pragma once

#include "CoreMinimal.h"
#include "ILive2DMotionEditor.h"
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Editor/PropertyEditor/Public/PropertyEditorDelegates.h"


class ULive2DModelMotion;
class IDetailsView;
class SDockableTab;
class UMyCustomAsset;

class LIVE2DMOTIONEDITOR_API FLive2DMotionEditor : public ILive2DMotionEditor
{
public:
	// This function creates tab spawners on editor initialization
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;

	// This function unregisters tab spawners on editor initialization
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;

	// This method decides how the custom asset editor will be initialized
	void InitLive2DMotionEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, ULive2DModelMotion* InLive2DMotion);

	/** Destructor */
	virtual ~FLive2DMotionEditor() override;

	/** Begin IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual bool IsPrimaryEditor() const override { return true; }
	/** End IToolkit interface */


	/** Begin ILive2DMotionEditor interface */
	virtual ULive2DModelMotion* GetLive2DModelMotion() const override;
	virtual void SetLive2DModelMotion(ULive2DModelMotion* InLive2DModelMotion) override;
	/** End ILive2DMotionEditor interface */

private:
	/** Create the properties tab and its content */
	TSharedRef<SDockTab> SpawnMotionPropertiesTab(const FSpawnTabArgs& Args);
	/** Create the properties tab and its content */
	TSharedRef<SDockTab> SpawnModelPropertiesTab(const FSpawnTabArgs& Args);
	/** Create the properties tab and its content */
	TSharedRef<SDockTab> SpawnLive2DMotionPreviewTab(const FSpawnTabArgs& Args);

	/** Dockable tab for properties */
	TSharedPtr< SDockableTab > PropertiesTab;

	/** Details view */
	TSharedPtr<class IDetailsView> MotionDetailsView;

	/** Details view */
	TSharedPtr<class IDetailsView> ModelDetailsView;

	/** Details view */
	TSharedPtr<class SLive2dModelImage> Live2DModelPreview;

	/**	The toolkit name */
	static const FName ToolkitFName;

	/**	The tab ids for all the tabs used */
	static const FName MotionPropertiesTabId;

	/**	The tab ids for all the tabs used */
	static const FName ModelPropertiesTabId;

	/**	The tab ids for all the tabs used */
	static const FName Live2DMotionPreviewTabId;

	/** The Custom Asset open within this editor */
	ULive2DModelMotion* Live2DModelMotion = nullptr;
	
	
};


