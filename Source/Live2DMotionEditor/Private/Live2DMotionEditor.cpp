#include "Live2DMotionEditor.h"
#include "Modules/ModuleManager.h"
#include "EditorStyleSet.h"
#include "Widgets/Docking/SDockTab.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Editor.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Live2DMotionEditorModule.h"
#include "Motion/Live2DModelMotion.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"

#define LOCTEXT_NAMESPACE "Live2DMotionEditor"

const FName FLive2DMotionEditor::ToolkitFName(TEXT("Live2DMotionEditor"));
const FName FLive2DMotionEditor::MotionPropertiesTabId(TEXT("Live2DMotionEditor_MotionProperties"));
const FName FLive2DMotionEditor::ModelPropertiesTabId(TEXT("Live2DMotionEditor_ModelProperties"));
const FName FLive2DMotionEditor::Live2DMotionPreviewTabId(TEXT("Live2DMotionEditor_MotionPreview"));

void FLive2DMotionEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	// Add a new workspace menu category to the tab manager
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_Live2DMotionEditor", "Live 2D Motion Editor"));

	// We register the tab manager to the asset editor toolkit so we can use it in this editor
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	// Register the properties tab spawner within our tab manager
	// We provide the function with the identifier for this tab and a shared pointer to the
	// SpawnPropertiesTab function within this editor class
	// Additionally, we provide a name to be displayed, a category and the tab icon
	InTabManager->RegisterTabSpawner(ModelPropertiesTabId, FOnSpawnTab::CreateSP(this, &FLive2DMotionEditor::SpawnModelPropertiesTab))
		.SetDisplayName(LOCTEXT("ModelPropertiesTab", "Model Details"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
	.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
	InTabManager->RegisterTabSpawner(MotionPropertiesTabId, FOnSpawnTab::CreateSP(this, &FLive2DMotionEditor::SpawnMotionPropertiesTab))
		.SetDisplayName(LOCTEXT("MotionPropertiesTab", "Motion Details"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
	InTabManager->RegisterTabSpawner(Live2DMotionPreviewTabId, FOnSpawnTab::CreateSP(this, &FLive2DMotionEditor::SpawnLive2DMotionPreviewTab))
		.SetDisplayName(LOCTEXT("Live2DMotionPreviewTab", "Motion Preview"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FLive2DMotionEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	// Unregister the tab manager from the asset editor toolkit
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	// Unregister our custom tab from the tab manager, making sure it is cleaned up when the editor gets destroyed
	InTabManager->UnregisterTabSpawner(MotionPropertiesTabId);
}

void FLive2DMotionEditor::InitLive2DMotionEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, ULive2DModelMotion* InLive2DMotion)
{
	// Cache some values that will be used for our details view arguments
	constexpr bool bIsUpdatable = false;
	constexpr bool bAllowFavorites = true;
	constexpr bool bIsLockable = false;

	// Set this InLive2DMocMotion as our editing asset
	SetLive2DModelMotion(InLive2DMotion);

	// Retrieve the property editor module and assign properties to DetailsView
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	const FDetailsViewArgs DetailsViewArgs(bIsUpdatable, bIsLockable, true, FDetailsViewArgs::ObjectsUseNameArea, false);
	MotionDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	ModelDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	SAssignNew(Live2DModelPreview, SImage).Image(&InLive2DMotion->GetModel()->GetTexture2DRenderTarget());

	// Create the layout of our custom asset editor
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_Live2DMotionEditor_Layout_v1")
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
				->AddTab(Live2DMotionPreviewTabId, ETabState::OpenedTab)
			)
			->Split
			(
				// Split the tab and pass the tab id to the tab spawner
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->AddTab(MotionPropertiesTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->AddTab(ModelPropertiesTabId, ETabState::OpenedTab)
				)
		)
		)
	);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	// Initialize our custom asset editor
	FAssetEditorToolkit::InitAssetEditor(
		Mode,
		InitToolkitHost,
		Live2DMotionEditorAppIdentifier,
		StandaloneDefaultLayout,
		bCreateDefaultStandaloneMenu,
		bCreateDefaultToolbar,
		(UObject*)InLive2DMotion);
    
	// Set the asset we are editing in the details view
	if (MotionDetailsView.IsValid())
	{
		MotionDetailsView->SetObject((UObject*)InLive2DMotion);
	}
	
	// Set the asset we are editing in the details view
	if (ModelDetailsView.IsValid())
	{
		ModelDetailsView->SetObject((UObject*)InLive2DMotion->GetModel());
	}
}

FLive2DMotionEditor::~FLive2DMotionEditor()
{
	// On destruction we reset our tab and details view 
	MotionDetailsView.Reset();
	PropertiesTab.Reset();
}

FName FLive2DMotionEditor::GetToolkitFName() const
{
	return ToolkitFName;
}

FText FLive2DMotionEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Live 2D Motion Editor");
}

FText FLive2DMotionEditor::GetToolkitName() const
{
	return FText::FromString(Live2DModelMotion->GetName());
}

FText FLive2DMotionEditor::GetToolkitToolTipText() const
{
	return LOCTEXT("ToolTip", "Live 2D Motion Editor to work with Live 2D Motions");
}

FString FLive2DMotionEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "AnimationDatabase ").ToString();
}

FLinearColor FLive2DMotionEditor::GetWorldCentricTabColorScale() const
{
	return FColor::Red;
}

ULive2DModelMotion* FLive2DMotionEditor::GetLive2DModelMotion() const
{
	return Live2DModelMotion;
;
}

void FLive2DMotionEditor::SetLive2DModelMotion(ULive2DModelMotion* InLive2DModelMotion)
{
	Live2DModelMotion = InLive2DModelMotion;
}

TSharedRef<SDockTab> FLive2DMotionEditor::SpawnMotionPropertiesTab(const FSpawnTabArgs& Args)
{
	// Make sure we have the correct tab id
	check(Args.GetTabId() == MotionPropertiesTabId);

	// Return a new slate dockable tab that contains our details view
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("GenericDetailsTitle", "Motion Details"))
		.TabColorScale(GetTabColorScale())
		[
				// Provide the details view as this tab its content
				MotionDetailsView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FLive2DMotionEditor::SpawnModelPropertiesTab(const FSpawnTabArgs& Args)
{
	
	// Make sure we have the correct tab id
	check(Args.GetTabId() == ModelPropertiesTabId);

	// Return a new slate dockable tab that contains our details view
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("GenericDetailsTitle", "Model Details"))
		.TabColorScale(GetTabColorScale())
		[
				// Provide the details view as this tab its content
				ModelDetailsView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FLive2DMotionEditor::SpawnLive2DMotionPreviewTab(const FSpawnTabArgs& Args)
{
	// Make sure we have the correct tab id
	check(Args.GetTabId() == Live2DMotionPreviewTabId);

	// Return a new slate dockable tab that contains our details view
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("GenericMotionPreviewTitle", "Motion Preview"))
		.TabColorScale(GetTabColorScale())
	[
		// Provide the preview as this tab its content
		Live2DModelPreview.ToSharedRef()
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
