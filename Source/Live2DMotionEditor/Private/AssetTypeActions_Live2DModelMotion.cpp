#include "AssetTypeActions_Live2DModelMotion.h"

#include "Live2DMotionEditorModule.h"
#include "Motion/Live2DModelMotion.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_Live2DModelMotion"

FText FAssetTypeActions_Live2DModelMotion::GetName() const
{
	return LOCTEXT("AssetTypeActions_Live2DModelMotion", "Live2DModel");
}

FColor FAssetTypeActions_Live2DModelMotion::GetTypeColor() const
{
	return FColor(125, 0, 219);
}

UClass* FAssetTypeActions_Live2DModelMotion::GetSupportedClass() const
{
	return ULive2DModelMotion::StaticClass();
}

void FAssetTypeActions_Live2DModelMotion::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Live2DModelMotion = Cast<ULive2DModelMotion>(*ObjIt);
		if (Live2DModelMotion != nullptr)
		{
			FLive2DMotionEditorModule* Live2DModelEditorModule = &FModuleManager::LoadModuleChecked<FLive2DMotionEditorModule>("Live2DMotionEditor");
			Live2DModelEditorModule->CreateLive2dMotionEditor(Mode, EditWithinLevelEditor, Live2DModelMotion);
		}
	}
}

uint32 FAssetTypeActions_Live2DModelMotion::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
