#pragma once

#include "CoreMinimal.h"
#include "Live2DEditorStructs.generated.h"


USTRUCT()
struct FModel3MotionFileData
{
	GENERATED_BODY()

	UPROPERTY()
	FString File;
};

USTRUCT()
struct FModel3MotionFileArrayData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FModel3MotionFileData> Motions;
};

USTRUCT()
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
