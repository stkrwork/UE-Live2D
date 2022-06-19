// Fill out your copyright notice in the Description page of Project Settings.


#include "Live2DMocModel.h"

#include "Live2DLogCategory.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Live2DCubismCore.h"

bool ULive2DMocModel::Init(const FString& FileName)
{	
	if (!FPaths::FileExists(FileName))
	{
		UE_LOG(LogLive2D, Error, TEXT("MOC3 file %s doesn't exist!"), *FileName);
		return false;
	}
	
	// Temporary buffer for the DNA file
	TArray<uint8> TempFileBuffer;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	IFileHandle* FileHandle = PlatformFile.OpenRead(*FileName);
	
	if (!FileHandle)
	{
		UE_LOG(LogLive2D, Error, TEXT("Couldn't open MOC3 file %s!"), *FileName);
		return false;
	}

	MocSourceSize = FileHandle->Size();

	MocSource = static_cast<uint8*>(FMemory::Malloc(MocSourceSize, csmAlignofMoc));

	FileHandle->Read(static_cast<uint8*>(MocSource), MocSourceSize);

	

	const csmMocVersion MocVersion = csmGetMocVersion(MocSource, MocSourceSize);

	if (csmGetLatestMocVersion() < MocVersion)
	{
		UE_LOG(LogLive2D, Error, TEXT("Can't load MOC3 file %s! The file version doesn't match with the currently used Live2D Cubism SDK version."), *FileName);
		return false;
	}

	if (!InitializeMoc())
	{
		UE_LOG(LogLive2D, Error, TEXT("Couldn't construct moc data structure from MOC3 file %s!"), *FileName);
		return false;
	}

	if (!InitializeModel())
	{
		UE_LOG(LogLive2D, Error, TEXT("Couldn't construct model from MOC3 file %s!"), *FileName);
		return false;
	}
	

	return true;
}

void ULive2DMocModel::BeginDestroy()
{
	FMemory::Free(Model);
	FMemory::Free(Moc);
	
	UObject::BeginDestroy();
}

void ULive2DMocModel::Serialize(FArchive& Ar)
{
	UObject::Serialize(Ar);

	if (Ar.IsLoading())
	{
		MocSource = static_cast<uint8*>(FMemory::Malloc(MocSourceSize, csmAlignofMoc));
	}
	
	Ar.Serialize(MocSource, MocSourceSize);

	if (Ar.IsLoading())
	{
		InitializeMoc();
		InitializeModel();
	}
}

FLive2DModelCanvasInfo ULive2DMocModel::GetModelCanvasInfo() const
{
	FLive2DModelCanvasInfo CanvasInfo;
	
	csmVector2 Size;
	csmVector2 PivotOrigin;

	csmReadCanvasInfo(Model, &Size, &PivotOrigin, &CanvasInfo.PixelsPerUnit);

	CanvasInfo.Size.X = Size.X;
	CanvasInfo.Size.Y = Size.Y;
	CanvasInfo.PivotOrigin.X = PivotOrigin.X;
	CanvasInfo.PivotOrigin.Y = PivotOrigin.Y;

	return CanvasInfo;
}

void ULive2DMocModel::UpdateDrawables()
{
	csmResetDrawableDynamicFlags(Model);
	csmUpdateModel(Model);
	
	int DrawableCount = csmGetDrawableCount(Model);
	const int* VertexCounts = csmGetDrawableVertexCounts(Model);
	const csmVector2** VertexPositions = csmGetDrawableVertexPositions(Model);
	const float* Opacities = csmGetDrawableOpacities(Model);
	const int* DrawOrders = csmGetDrawableDrawOrders(Model);
	const int* RenderOrders = csmGetDrawableRenderOrders(Model);
	const csmFlags* DynamicFlags = csmGetDrawableDynamicFlags(Model);

	for (int32 ModelDrawableIndex = 0; ModelDrawableIndex < DrawableCount; ModelDrawableIndex++)
	{
		FLive2DModelDrawable& Drawable = Drawables[ModelDrawableIndex];

		const int32 VertexCount = VertexCounts[ModelDrawableIndex];
		Drawable.VertexPositions.SetNum(VertexCount);

		for (int VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
		{
			Drawable.VertexPositions[VertexIndex].X = VertexPositions[ModelDrawableIndex][VertexIndex].X;
			Drawable.VertexPositions[VertexIndex].Y = VertexPositions[ModelDrawableIndex][VertexIndex].Y;
		}

		
		// Access to other Drawable elements
		Drawable.DrawOrder = DrawOrders[ModelDrawableIndex];

		// The following three items are important on rendering.
		Drawable.Opacity = Opacities[ModelDrawableIndex];
		Drawable.RenderOrder = RenderOrders[ModelDrawableIndex];
		Drawable.DynamicFlag = DynamicFlags[ModelDrawableIndex];
	}
}

float ULive2DMocModel::GetParameterValue(const FString& ParameterName)
{
	auto* ParameterValue = ParameterValues.Find(ParameterName);

	if (!ParameterValue)
	{
		UE_LOG(LogLive2D, Error, TEXT("ULive2DMocModel::GetParameterValue: Parameter doesn't exist on Live 2D Model!"));
		return 0.f;
	}

	return **ParameterValue;
}

void ULive2DMocModel::SetParameterValue(const FString& ParameterName, const float Value, const bool bUpdateDrawables)
{
	auto* ParameterValue = ParameterValues.Find(ParameterName);

	if (!ParameterValue)
	{
		UE_LOG(LogLive2D, Error, TEXT("ULive2DMocModel::SetParameterValue: Parameter doesn't exist on Live 2D Model!"));
		return;
	}

	**ParameterValue = Value;

	if (bUpdateDrawables)
	{
		UpdateDrawables();
	}
}

bool ULive2DMocModel::InitializeMoc()
{
	Moc = csmReviveMocInPlace(MocSource, MocSourceSize);
	return Moc != nullptr;
}

bool ULive2DMocModel::InitializeModel()
{
	uint32 ModelSize = csmGetSizeofModel(Moc);

	void* ModelMemoryAligned = FMemory::Malloc(ModelSize, csmAlignofModel);

	Model = csmInitializeModelInPlace(Moc, ModelMemoryAligned, ModelSize);

	if (Model)
	{
		InitializeParameterList();
		InitializeDrawables();
	}
	
	return Model != nullptr;
}

void ULive2DMocModel::InitializeParameterList()
{
	const int32 ModelParameterCount = csmGetParameterCount(Model);
	const char** ModelParameterIds = csmGetParameterIds(Model);
	float* ModelParameterValues = csmGetParameterValues(Model);
	const float* ModelParameterMaximumValues = csmGetParameterMaximumValues(Model);
	const float* ModelParameterMinimumValues = csmGetParameterMinimumValues(Model);
	const float* ModelParameterDefaultValues = csmGetParameterDefaultValues(Model);

	for(int32 ParameterIndex = 0; ParameterIndex < ModelParameterCount; ParameterIndex++)
	{
		const FString ParameterId = ModelParameterIds[ParameterIndex];
		ParameterValues.Add(ParameterId, ModelParameterValues + ParameterIndex);
		ParameterDefaultValues.Add(ParameterId, ModelParameterDefaultValues[ParameterIndex]);
		ParameterMinimumValues.Add(ParameterId, ModelParameterMinimumValues[ParameterIndex]);
		ParameterMaximumValues.Add(ParameterId, ModelParameterMaximumValues[ParameterIndex]);
	}
}

void ULive2DMocModel::InitializeDrawables()
{
	auto DrawableCount= csmGetDrawableCount(Model);
	Drawables.SetNum(DrawableCount);
	
	const int* TextureIndices = csmGetDrawableTextureIndices(Model);
	const csmFlags* ConstantFlags = csmGetDrawableConstantFlags(Model);
	const int* VertexCounts = csmGetDrawableVertexCounts(Model);
	const csmVector2** VertexPositions = csmGetDrawableVertexPositions(Model);
	const csmVector2** VertexUvs = csmGetDrawableVertexUvs(Model);
	const int* IndexCounts = csmGetDrawableIndexCounts(Model);
	const unsigned short** VertexIndices = csmGetDrawableIndices(Model);
	const char** Ids = csmGetDrawableIds(Model);
	const float* Opacities = csmGetDrawableOpacities(Model);
	const int* DrawOrders = csmGetDrawableDrawOrders(Model);
	const int* RenderOrders = csmGetDrawableRenderOrders(Model);
	const csmFlags* DynamicFlags = csmGetDrawableDynamicFlags(Model);
	const int* MaskCounts = csmGetDrawableMaskCounts(Model);
	const int** Masks = csmGetDrawableMasks(Model);

	for (int32 ModelDrawableIndex = 0; ModelDrawableIndex < DrawableCount; ModelDrawableIndex++)
	{
		FLive2DModelDrawable& Drawable = Drawables[ModelDrawableIndex];
		Drawable.TextureIndex = TextureIndices[ModelDrawableIndex];

		if ((ConstantFlags[ModelDrawableIndex] & csmBlendAdditive) == csmBlendAdditive)
		{
			Drawable.BlendMode = ELive2dModelBlendMode::ADDITIVE_BLENDING;
		}
		else if ((ConstantFlags[ModelDrawableIndex] & csmBlendMultiplicative) == csmBlendMultiplicative)
		{
			Drawable.BlendMode = ELive2dModelBlendMode::MULTIPLICATIVE_BLENDING;
		}
		else
		{
			Drawable.BlendMode = ELive2dModelBlendMode::NORMAL_BLENDING;
		}

		Drawable.bIsDoubleSided = (ConstantFlags[ModelDrawableIndex] & csmIsDoubleSided) == csmIsDoubleSided;
		Drawable.bIsInvertedMask = (ConstantFlags[ModelDrawableIndex] & csmIsInvertedMask) == csmIsDoubleSided;

		const int32 VertexCount = VertexCounts[ModelDrawableIndex];
		Drawable.VertexPositions.SetNum(VertexCount);
		Drawable.VertexUVs.SetNum(VertexCount);

		for (int VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
		{
			Drawable.VertexPositions[VertexIndex].X = VertexPositions[ModelDrawableIndex][VertexIndex].X;
			Drawable.VertexPositions[VertexIndex].Y = VertexPositions[ModelDrawableIndex][VertexIndex].Y;

			Drawable.VertexUVs[VertexIndex].X = VertexUvs[ModelDrawableIndex][VertexIndex].X;
			Drawable.VertexUVs[VertexIndex].Y = VertexUvs[ModelDrawableIndex][VertexIndex].Y;
		}

		const int32 IndexCount = IndexCounts[ModelDrawableIndex];
		Drawable.VertexIndices.SetNum(IndexCount);

		for (int32 Index = 0; Index < IndexCount; Index++)
		{
			Drawable.VertexIndices[Index] = VertexIndices[ModelDrawableIndex][Index];
		}
		
		// Access to other Drawable elements
		Drawable.ID = Ids[ModelDrawableIndex];
		Drawable.DrawOrder = DrawOrders[ModelDrawableIndex];

		// The following three items are important on rendering.
		Drawable.Opacity = Opacities[ModelDrawableIndex];
		Drawable.RenderOrder = RenderOrders[ModelDrawableIndex];
		Drawable.DynamicFlag = DynamicFlags[ModelDrawableIndex];
		const int32 MaskCount = MaskCounts[ModelDrawableIndex];
		Drawable.Masks.SetNum(MaskCount);
		for (int32 MaskIndex = 0; MaskIndex < MaskCount; MaskIndex++)
		{
			Drawable.Masks[MaskIndex] = Masks[ModelDrawableIndex][MaskIndex];
			// Numbers in masks are index of Drawable
			//Drawable.MaskLinks = &Drawables[Masks[ModelDrawableIndex][MaskIndex]];
		}
	}
}
