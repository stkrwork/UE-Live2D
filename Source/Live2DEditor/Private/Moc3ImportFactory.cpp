// Fill out your copyright notice in the Description page of Project Settings.


#include "Moc3ImportFactory.h"

#include "Editor.h"
#include "Live2DEditorLogCategory.h"
#include "Live2DMocModel.h"
#include "Misc/MessageDialog.h"

UMoc3ImportFactory::UMoc3ImportFactory()
	:Super()
{	
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = ULive2DMocModel::StaticClass();

	bEditorImport = true;
	bText = false;

	Formats.Add(TEXT("moc3;Live2D Model Information file"));
}

UObject* UMoc3ImportFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, *Filename, TEXT("moc3"));
			
	ULive2DMocModel* MocModel = NewObject< ULive2DMocModel >(InParent, InName, RF_Public | RF_Standalone | RF_Transactional | RF_LoadCompleted);
	if (MocModel->Init(*Filename))
	{
		GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, MocModel);
		return MocModel;
	}
	
	const FText Message = NSLOCTEXT("Live2DEditor", "Moc3ImportFailed", "Moc3 file can not imported.");
	FMessageDialog::Open(EAppMsgType::Ok, Message);
	UE_LOG(LogLive2DEditor, Warning, TEXT("%s"), *Message.ToString());
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, nullptr);

	return nullptr;
	
}

bool UMoc3ImportFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);

	if (Extension == TEXT("moc3"))
	{
		return true;
	}

	return false;
}

void UMoc3ImportFactory::CleanUp()
{
	Super::CleanUp();
}
