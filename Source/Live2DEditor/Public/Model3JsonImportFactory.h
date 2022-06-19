// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Model3JsonImportFactory.generated.h"

/**
 * 
 */
UCLASS()
class LIVE2DEDITOR_API UModel3JsonImportFactory : public UFactory
{
	GENERATED_BODY()

public:
	UModel3JsonImportFactory();
	
	/** UFactory interface */
	virtual UObject* FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual void CleanUp() override;
};
