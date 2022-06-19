// Fill out your copyright notice in the Description page of Project Settings.


#include "Model3JsonImportFactory.h"

#include "Editor.h"
#include "ImageUtils.h"
#include "Live2DEditorLogCategory.h"
#include "Live2DMocModel.h"
#include "Misc/MessageDialog.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "Live2DEditorStructs.h"
#include "Live2DEditorUtils.h"
#include "Factories/Texture2dFactoryNew.h"
#include "Factories/TextureFactory.h"

UModel3JsonImportFactory::UModel3JsonImportFactory()
	:Super()
{	
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = ULive2DMocModel::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("model3;Live2D Model Json Information file"));
}

UObject* UModel3JsonImportFactory::FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	FModel3Data Model3Data = Live2DEditorUtils::CreateModel3DataFromJsonString(Buffer);

	const FString Path = FPaths::GetPath(CurrentFilename);
	
	ULive2DMocModel* MocModel = NewObject< ULive2DMocModel >(InParent, InName, Flags);
	
	if (MocModel->Init(Path / Model3Data.Moc))
	{
		UTextureFactory* Factory = NewObject<UTextureFactory>();
		for (const auto& TextureName: Model3Data.Textures)
		{
			const FName TextureAssetName = *TextureName.Replace(TEXT("/"), TEXT("_")).Replace(TEXT("."), TEXT("_"));
			UTexture2D* NewTexture = Cast<UTexture2D>(Factory->FactoryCreateFile(UTexture2D::StaticClass(), InParent, TextureAssetName, Flags, Path / TextureName, 0, Warn,bOutOperationCanceled));
			MocModel->Textures.Add(NewTexture);
		}
		
		return MocModel;
	}
	
	const FText Message = NSLOCTEXT("Live2DEditor", "Moc3ImportFailed", "Moc3 file can not imported.");
	FMessageDialog::Open(EAppMsgType::Ok, Message);
	UE_LOG(LogLive2DEditor, Warning, TEXT("%s"), *Message.ToString());

	return nullptr;
	
}

bool UModel3JsonImportFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);

	if (Extension == TEXT("model3"))
	{
		return true;
	}

	return false;
}

void UModel3JsonImportFactory::CleanUp()
{
	Super::CleanUp();
}
