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

USTRUCT(BlueprintType)
struct FMotion3CurveData
{
	GENERATED_BODY()

	UPROPERTY()
	FString Target;

	UPROPERTY()
	FString Id;

	UPROPERTY()
	float FadeInTime = -1.f;

	UPROPERTY()
	float FadeOutTime = -1.f;

	UPROPERTY()
	TArray<float> Segments;
};
USTRUCT(BlueprintType)
struct FModel3MotionFileData
{
	GENERATED_BODY()

	UPROPERTY()
	FString File;
};

USTRUCT(BlueprintType)
struct FModel3MotionFileArrayData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FModel3MotionFileData> Motions;
};

USTRUCT(BlueprintType)
struct FModel3Data
{
	GENERATED_BODY()

	UPROPERTY()
	FString Moc;
	
	UPROPERTY()
	TArray<FString> Textures;
	
	UPROPERTY()
	FString Physics;
	
	UPROPERTY()
	FString DisplayInfo;

	UPROPERTY()
	TMap<FString, FModel3MotionFileArrayData> Motions;
	
};

USTRUCT(BlueprintType)
struct FMotion3UserData
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	FString Value;
};
USTRUCT(BlueprintType)
struct FMotion3MetaData
{
	GENERATED_BODY()

	UPROPERTY()
	float Duration;

	UPROPERTY()
	float FPS;

	UPROPERTY()
	bool bLoop;

	UPROPERTY()
	bool bAreBeziersRestricted;

	UPROPERTY()
	int32 CurveCount;

	UPROPERTY()
	int32 TotalSegmentCount;

	UPROPERTY()
	int32 TotalPointCount;

	UPROPERTY()
	int32 UserDataCount;

	UPROPERTY()
	int32 TotalUserDataSize;
};

USTRUCT(BlueprintType)
struct FMotion3FileData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Version;

	UPROPERTY()
	FMotion3MetaData Meta;

	UPROPERTY()
	TArray<FMotion3UserData> UserData; 

	UPROPERTY()
	TArray<FMotion3CurveData> Curves; 
};