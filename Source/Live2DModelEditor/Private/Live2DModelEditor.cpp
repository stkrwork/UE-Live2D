#include "Live2DModelEditor.h"
#include "Modules/ModuleManager.h"
#include "EditorStyleSet.h"
#include "Widgets/Docking/SDockTab.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Editor.h"
#include "Live2DMocModel.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Live2DModelEditorModule.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"

#define LOCTEXT_NAMESPACE "Live2DModelEditor"

const FName FLive2DModelEditor::ToolkitFName(TEXT("Live2DModelEditor"));
const FName FLive2DModelEditor::PropertiesTabId(TEXT("Live2DModelEditor_Properties"));
const FName FLive2DModelEditor::Live2DModelPreviewTabId(TEXT("Live2DModelEditor_ModelPreview"));

void FLive2DModelEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	// Add a new workspace menu category to the tab manager
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_Live2DModelEditor", "Live 2D Model Editor"));

	// We register the tab manager to the asset editor toolkit so we can use it in this editor
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	// Register the properties tab spawner within our tab manager
	// We provide the function with the identifier for this tab and a shared pointer to the
	// SpawnPropertiesTab function within this editor class
	// Additionally, we provide a name to be displayed, a category and the tab icon
	InTabManager->RegisterTabSpawner(PropertiesTabId, FOnSpawnTab::CreateSP(this, &FLive2DModelEditor::SpawnPropertiesTab))
		.SetDisplayName(LOCTEXT("PropertiesTab", "Details"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
	InTabManager->RegisterTabSpawner(Live2DModelPreviewTabId, FOnSpawnTab::CreateSP(this, &FLive2DModelEditor::SpawnLive2DModelPreviewTab))
		.SetDisplayName(LOCTEXT("Live2DModelPreviewTab", "Model Preview"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FLive2DModelEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	// Unregister the tab manager from the asset editor toolkit
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	// Unregister our custom tab from the tab manager, making sure it is cleaned up when the editor gets destroyed
	InTabManager->UnregisterTabSpawner(PropertiesTabId);
}

void FLive2DModelEditor::InitLive2DModelEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, ULive2DMocModel* InLive2DMocModel)
{
	// Cache some values that will be used for our details view arguments
	constexpr bool bIsUpdatable = false;
	constexpr bool bAllowFavorites = true;
	constexpr bool bIsLockable = false;

	// Set this InLive2DMocModel as our editing asset
	SetLive2DMocModel(InLive2DMocModel);

	// Retrieve the property editor module and assign properties to DetailsView
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	const FDetailsViewArgs DetailsViewArgs(bIsUpdatable, bIsLockable, true, FDetailsViewArgs::ObjectsUseNameArea, false);
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	SAssignNew(Live2DModelPreview, SImage).Image(&InLive2DMocModel->GetImageBrush());

	// Create the layout of our custom asset editor
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_Live2DModelEditor_Layout_v1")
	->AddArea
	(
		// Create a vertical area and spawn the toolbar
		FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.1f)
			->SetHideTabWell(true)
			->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
		)
		->Split
		(
			// Split the tab and pass the tab id to the tab spawner
			FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(Live2DModelPreviewTabId, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(PropertiesTabId, ETabState::OpenedTab)
			)
		)
	);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	// Initialize our custom asset editor
	FAssetEditorToolkit::InitAssetEditor(
		Mode,
		InitToolkitHost,
		Live2DModelEditorAppIdentifier,
		StandaloneDefaultLayout,
		bCreateDefaultStandaloneMenu,
		bCreateDefaultToolbar,
		(UObject*)InLive2DMocModel);
    
	// Set the asset we are editing in the details view
	if (DetailsView.IsValid())
	{
		DetailsView->SetObject((UObject*)InLive2DMocModel);
	}
}

FLive2DModelEditor::~FLive2DModelEditor()
{
	// On destruction we reset our tab and details view 
	DetailsView.Reset();
	PropertiesTab.Reset();
}

FName FLive2DModelEditor::GetToolkitFName() const
{
	return ToolkitFName;
}

FText FLive2DModelEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Live 2D Model Editor");
}

FText FLive2DModelEditor::GetToolkitName() const
{
	return FText::FromString(Live2DMocModel->GetName());
}

FText FLive2DModelEditor::GetToolkitToolTipText() const
{
	return LOCTEXT("ToolTip", "Live 2D Model Editor to work with Live 2D Models");
}

FString FLive2DModelEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "AnimationDatabase ").ToString();
}

FLinearColor FLive2DModelEditor::GetWorldCentricTabColorScale() const
{
	return FColor::Red;
}

ULive2DMocModel* FLive2DModelEditor::GetLive2DMocModel() const
{
	return Live2DMocModel;
}

void FLive2DModelEditor::SetLive2DMocModel(ULive2DMocModel* InLive2DMocModel)
{
	Live2DMocModel = InLive2DMocModel;
}

TSharedRef<SDockTab> FLive2DModelEditor::SpawnPropertiesTab(const FSpawnTabArgs& Args)
{
	// Make sure we have the correct tab id
	check(Args.GetTabId() == PropertiesTabId);

	// Return a new slate dockable tab that contains our details view
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("GenericDetailsTitle", "Details"))
		.TabColorScale(GetTabColorScale())
		[
				// Provide the details view as this tab its content
				DetailsView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FLive2DModelEditor::SpawnLive2DModelPreviewTab(const FSpawnTabArgs& Args)
{
	// Make sure we have the correct tab id
	check(Args.GetTabId() == Live2DModelPreviewTabId);

	// Return a new slate dockable tab that contains our details view
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("GenericMotionPreviewTitle", "Motion Preview"))
		.TabColorScale(GetTabColorScale())
	[
		SNew(SScaleBox)
		.Stretch(EStretch::ScaleToFit)
		.StretchDirection(EStretchDirection::Both)
		[
		// Provide the preview as this tab its content
			Live2DModelPreview.ToSharedRef()
		]
		// SNew(SConstraintCanvas)
		// + SConstraintCanvas::Slot()
		// .AutoSize(true)
		// .Anchors(FAnchors(0.5f))
		// [
		// 	SNew(SConstraintCanvas)
		// 	+ SConstraintCanvas::Slot()
		// 	.AutoSize(true)
		// 	[
		// 		SNew(SScaleBox)
		// 		.Stretch(EStretch::ScaleToFit)
		// 		.StretchDirection(EStretchDirection::Both)
		// 		[
		// 			SNew(SConstraintCanvas)
		// 			+ SConstraintCanvas::Slot()
		// 			.Anchors(FAnchors(0.f, 0.f, 1.f, 1.f))
		// 			[
		// 				// Provide the preview as this tab its content
		// 				SNew(SImage)
		// 				.Image(FEditorStyle::GetBrush(TEXT("Checkerboard")))
		// 			]
		// 			+ SConstraintCanvas::Slot()
		// 			.AutoSize(true)
		// 			.Anchors(FAnchors(0.5f))
		// 			.Alignment(0.5f)
		// 			[
		// 				// Provide the preview as this tab its content
		// 				Live2DModelPreview.ToSharedRef()	
		// 			]
		// 		]
		// 	]
		// ]
	];	
}

#undef LOCTEXT_NAMESPACE
