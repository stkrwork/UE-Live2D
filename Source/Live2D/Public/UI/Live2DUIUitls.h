// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Live2DModelMotion.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Live2DUIUitls.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class LIVE2D_API ULive2DUIUitls : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Live 2D")
	static void SetBrushFromLive2DModelMotion(UImage* Image, ULive2DModelMotion* ModelMotion);
	
	UFUNCTION(BlueprintCallable, Category="Live 2D")
	static void SetBrushFromSoftLive2DModelMotion(UImage* Image, TSoftObjectPtr<ULive2DModelMotion> ModelMotion);
};
