// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Live2DCubismCore.h"
#include "Live2DStructs.h"
#include "CoreUObject/Public/UObject/Object.h"
#include "Engine/Texture2D.h"
#include "Live2DMocModel.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LIVE2D_API ULive2DMocModel : public UObject
{
	GENERATED_BODY()

public:	
	bool Init(const FString& FileName);

	virtual void BeginDestroy() override;

	virtual void Serialize(FArchive& Ar) override;

	FLive2DModelCanvasInfo GetModelCanvasInfo() const;

	void UpdateDrawables();

	float GetParameterValue(const FString& ParameterName);
	void SetParameterValue(const FString& ParameterName, const float Value, const bool bUpdateDrawables = false);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FLive2DModelDrawable> Drawables;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UTexture2D*> Textures;
	
private:

	bool InitializeMoc();
	bool InitializeModel();
	void InitializeParameterList();
	void InitializeDrawables();

	UPROPERTY()
	int32 MocSourceSize;

	TMap<FString, float*> ParameterValues;
	TMap<FString, float> ParameterDefaultValues;
	TMap<FString, float> ParameterMaximumValues;
	TMap<FString, float> ParameterMinimumValues;

	uint8* MocSource;
	csmMoc* Moc;
	csmModel* Model;
};
