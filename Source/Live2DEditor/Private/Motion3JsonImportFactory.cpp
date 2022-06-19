// Fill out your copyright notice in the Description page of Project Settings.


#include "Motion3JsonImportFactory.h"

#include "Live2DEditorLogCategory.h"
#include "Misc/MessageDialog.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "Live2DEditorUtils.h"
#include "Factories/Texture2dFactoryNew.h"
#include "Factories/TextureFactory.h"
#include "Motion/Live2DModelMotion.h"

UMotion3JsonImportFactory::UMotion3JsonImportFactory()
	:Super()
{	
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = ULive2DModelMotion::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("motion3;Live2D Motion Json Information file"));
}

UObject* UMotion3JsonImportFactory::FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	FMotion3FileData Motion3Data = Live2DEditorUtils::CreateMotion3FileDataFromJsonString(Buffer);

	const FString Path = FPaths::GetPath(CurrentFilename);
	
	ULive2DModelMotion* MocMotion = NewObject< ULive2DModelMotion >(InParent, InName, Flags);
	
	if (MocMotion->Init(Motion3Data))
	{		
		return MocMotion;
	}
	
	const FText Message = NSLOCTEXT("Live2DEditor", "Moc3ImportFailed", "Moc3 file can not imported.");
	FMessageDialog::Open(EAppMsgType::Ok, Message);
	UE_LOG(LogLive2DEditor, Warning, TEXT("%s"), *Message.ToString());

	return nullptr;
	
}

bool UMotion3JsonImportFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);

	if (Extension == TEXT("motion3"))
	{
		return true;
	}

	return false;
}

void UMotion3JsonImportFactory::CleanUp()
{
	Super::CleanUp();
}
