// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Moc3ImportFactory.generated.h"

/**
 * 
 */
UCLASS()
class LIVE2DEDITOR_API UMoc3ImportFactory : public UFactory
{
	GENERATED_BODY()

public:
	UMoc3ImportFactory();
	
	/** UFactory interface */
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual void CleanUp() override;
};
