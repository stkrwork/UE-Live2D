#include "AssetTypeActions_Live2DMocModel.h"
#include "Live2DMocModel.h"
#include "Live2DModelEditorModule.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_Live2DMocModel"

FText FAssetTypeActions_Live2DMocModel::GetName() const
{
	return LOCTEXT("AssetTypeActions_Live2DMocModel", "Live2DModel");
}

FColor FAssetTypeActions_Live2DMocModel::GetTypeColor() const
{
	return FColor(0, 169, 224);
}

UClass* FAssetTypeActions_Live2DMocModel::GetSupportedClass() const
{
	return ULive2DMocModel::StaticClass();
}

void FAssetTypeActions_Live2DMocModel::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Live2DMocModel = Cast<ULive2DMocModel>(*ObjIt);
		if (Live2DMocModel != NULL)
		{
			FLive2DModelEditorModule* Live2DModelEditorModule = &FModuleManager::LoadModuleChecked<FLive2DModelEditorModule>("Live2DModelEditor");
			Live2DModelEditorModule->CreateLive2dModelEditor(Mode, EditWithinLevelEditor, Live2DMocModel);
		}
	}
}

uint32 FAssetTypeActions_Live2DMocModel::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
