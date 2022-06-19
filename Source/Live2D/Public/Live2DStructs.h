#pragma once

#include "CoreMinimal.h"
#include "Live2DCubismCore.h"
#include "Live2DStructs.generated.h"

USTRUCT(BlueprintType)
struct FLive2DModelCanvasInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D Size;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D PivotOrigin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PixelsPerUnit;
};

UENUM(BlueprintType)
enum class ELive2dModelBlendMode : uint8
{
	INVALID_BLEND_MODE,
	ADDITIVE_BLENDING,
	MULTIPLICATIVE_BLENDING,
	NORMAL_BLENDING
};

USTRUCT(BlueprintType)
struct FLive2DModelDrawable
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 TextureIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ELive2dModelBlendMode BlendMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsDoubleSided;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsInvertedMask;
	TArray<FVector2D> VertexPositions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FVector2D> VertexUVs;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<int32> VertexIndices;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString ID;
	int32 DrawOrder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Opacity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 RenderOrder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 DynamicFlag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<int32> Masks;

	bool IsVisible() const
	{
		return (DynamicFlag & csmIsVisible) == csmIsVisible;
	}
	
};

USTRUCT()
struct FLive2DModelPart
{
	GENERATED_BODY()

	FString ID;

	float* Opacity;
};
