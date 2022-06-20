// Fill out your copyright notice in the Description page of Project Settings.


#include "Live2DMocModel.h"

#include "CanvasItem.h"
#include "Live2DLogCategory.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Live2DCubismCore.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetRenderingLibrary.h"

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

	uint8* Source = static_cast<uint8*>(FMemory::Malloc(MocSourceSize, csmAlignofMoc));
	MocSource = static_cast<uint8*>(FMemory::Malloc(MocSourceSize, csmAlignofMoc));


	FileHandle->Read(Source, MocSourceSize);
	FMemory::Memcpy(MocSource, Source, MocSourceSize);
	
	const csmMocVersion MocVersion = csmGetMocVersion(MocSource, MocSourceSize);

	if (csmGetLatestMocVersion() < MocVersion)
	{
		UE_LOG(LogLive2D, Error, TEXT("Can't load MOC3 file %s! The file version doesn't match with the currently used Live2D Cubism SDK version."), *FileName);
		return false;
	}

	if (!InitializeMoc(Source))
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
		uint8* Source = static_cast<uint8*>(FMemory::Malloc(MocSourceSize, csmAlignofMoc));
		FMemory::Memcpy(Source, MocSource, MocSourceSize);
		InitializeMoc(Source);
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

	Drawables.Sort([](const FLive2DModelDrawable& l, const FLive2DModelDrawable& r)
	{
		return (l.RenderOrder < r.RenderOrder);
	});
	
	OnDrawablesUpdated.Broadcast();
}

float ULive2DMocModel::GetParameterValue(const FString& ParameterName)
{
	auto* ParameterValue = ParameterValues.Find(ParameterName);

	if (!ParameterValue)
	{
		UE_LOG(LogLive2D, Error, TEXT("ULive2DMocModel::GetParameterValue: Parameter %s doesn't exist on Live 2D Model!"), *ParameterName);
		return 0.f;
	}

	return *ParameterValue;
}

void ULive2DMocModel::SetParameterValue(const FString& ParameterName, const float Value, const bool bUpdateDrawables)
{
	auto* ParameterValue = ParameterValues.Find(ParameterName);

	if (!ParameterValue)
	{
		UE_LOG(LogLive2D, Error, TEXT("ULive2DMocModel::SetParameterValue: Parameter %s doesn't exist on Live 2D Model!"), *ParameterName);
		return;
	}

	
	auto* parameterIds = csmGetParameterIds(Model);
	auto* parameterValues = csmGetParameterValues(Model);
	// Scan array position corresponding to target ID
	int32 targetIndex = -1;
	for(int32 i = 0; i < ParameterValues.Num() ;++i)
	{
		if( ParameterName.Compare(parameterIds[i]) == 0 )
		{
			targetIndex = i;
			break;
		}
	}
	//In case that the desired ID could n't be found ID
	if(targetIndex == -1 )
	{
		return;
	}

	const float MinValue = ParameterMinimumValues[ParameterName];
	const float MaxValue = ParameterMaximumValues[ParameterName];

	if (MinValue > Value)
	{
		parameterValues[targetIndex] = MinValue;
		*ParameterValue = MinValue;
	}
	else if (MaxValue < Value)
	{
		parameterValues[targetIndex] = MaxValue;
		*ParameterValue = MaxValue;
	}
	else
	{
		parameterValues[targetIndex] = Value;
		*ParameterValue = Value;
	}
	
	
	if (bUpdateDrawables)
	{
		UpdateDrawables();
	}
}

float ULive2DMocModel::GetPartOpacityValue(const FString& ParameterName)
{
	auto* PartOpacity = PartOpacities.Find(ParameterName);

	if (!PartOpacity)
	{
		UE_LOG(LogLive2D, Error, TEXT("ULive2DMocModel::GetPartOpacityValue: Part Opacity Parameter %s doesn't exist on Live 2D Model!"), *ParameterName);
		return 0.f;
	}

	return *PartOpacity;
}

void ULive2DMocModel::SetPartOpacityValue(const FString& ParameterName, const float Value, const bool bUpdateDrawables)
{
	auto* PartOpacity = PartOpacities.Find(ParameterName);

	if (!PartOpacity)
	{
		UE_LOG(LogLive2D, Error, TEXT("ULive2DMocModel::SetPartOpacityValue: Part Opacity Parameter %s doesn't exist on Live 2D Model!"), *ParameterName);
		return;
	}

	
	const char** ModelPartIds = csmGetPartIds(Model);
	float* ModelPartOpacities = csmGetPartOpacities(Model);
	// Scan array position corresponding to target ID
	int32 targetIndex = -1;
	for(int32 i = 0; i < ParameterValues.Num() ;++i)
	{
		if( ParameterName.Compare(ModelPartIds[i]) == 0 )
		{
			targetIndex = i;
			break;
		}
	}
	//In case that the desired ID could n't be found ID
	if(targetIndex == -1 )
	{
		return;
	}
	
	//Multiply the difference from reference value by the specified magnification ratio from the parameter.
	ModelPartOpacities[targetIndex] = Value;
	*PartOpacity = Value;
	
	if (bUpdateDrawables)
	{
		UpdateDrawables();
	}
}

FSlateBrush& ULive2DMocModel::GetTexture2DRenderTarget()
{
	auto CanvasInfo = GetModelCanvasInfo();
	RenderTarget2D = NewObject<UTextureRenderTarget2D>(this);
	check(RenderTarget2D);
	RenderTarget2D->RenderTargetFormat = RTF_RGBA16f;
	RenderTarget2D->ClearColor = FLinearColor::Transparent;
	RenderTarget2D->bAutoGenerateMips = true;
	RenderTarget2D->InitAutoFormat(CanvasInfo.Size.X, CanvasInfo.Size.Y);	
	RenderTarget2D->UpdateResourceImmediate(true);

	UCanvas* Canvas;

	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(GWorld, RenderTarget2D, Canvas, CanvasInfo.Size, Context);

	for (const auto& Drawable: Drawables)
	{
		TArray<FCanvasUVTri> TriangleList;

		for (int32 i = 0; i < Drawable.VertexIndices.Num(); i += 3)
		{
			const int32 VertexIndex0 = Drawable.VertexIndices[i];
			const int32 VertexIndex1 = Drawable.VertexIndices[i+1];
			const int32 VertexIndex2 = Drawable.VertexIndices[i+2];
			
			FCanvasUVTri Triangle;
			Triangle.V0_Pos = ProcessVertex(Drawable.VertexPositions[VertexIndex0], CanvasInfo);
			Triangle.V1_Pos = ProcessVertex(Drawable.VertexPositions[VertexIndex1], CanvasInfo);
			Triangle.V2_Pos = ProcessVertex(Drawable.VertexPositions[VertexIndex2], CanvasInfo);
			Triangle.V0_UV = Drawable.VertexUVs[VertexIndex0];
			Triangle.V0_UV.Y = 1 - Triangle.V0_UV.Y;
			Triangle.V1_UV = Drawable.VertexUVs[VertexIndex1];
			Triangle.V1_UV.Y = 1 - Triangle.V1_UV.Y;
			Triangle.V2_UV = Drawable.VertexUVs[VertexIndex2];
			Triangle.V2_UV.Y = 1 - Triangle.V2_UV.Y;
			Triangle.V0_Color = FLinearColor::White;
			Triangle.V1_Color = FLinearColor::White;
			Triangle.V2_Color = FLinearColor::White;

			TriangleList.Add(Triangle);
		}
		
		FCanvasTriangleItem TriangleItem(TriangleList, Textures[Drawable.TextureIndex]->GetResource());
		TriangleItem.BlendMode = SE_BLEND_Masked;

		Canvas->DrawItem(TriangleItem);
	}

	RenderTargetBrush.SetResourceObject(RenderTarget2D);
	
	return RenderTargetBrush; 
}

FVector2D ULive2DMocModel::ProcessVertex(FVector2D Vertex, const FLive2DModelCanvasInfo& CanvasInfo)
{
	FVector2D CanvasCenter(CanvasInfo.PivotOrigin.X/CanvasInfo.PixelsPerUnit,CanvasInfo.PivotOrigin.Y/CanvasInfo.PixelsPerUnit);
	FVector2D CanvasDimensions(CanvasInfo.Size.X/CanvasInfo.PixelsPerUnit,CanvasInfo.Size.Y/CanvasInfo.PixelsPerUnit);
	Vertex *= CanvasInfo.Size;
	Vertex.X += (CanvasInfo.Size.X * CanvasCenter.X);
	Vertex.Y += (CanvasInfo.Size.Y * CanvasCenter.Y);

	// TODO move vertices to correct position so nothing gets rendered off-screen
	if (CanvasCenter.X != 0.5f)
	{
		Vertex.X += CanvasInfo.Size.X * (CanvasCenter.X - 0.5f);
	}
	
	// TODO move vertices to correct position so nothing gets rendered off-screen
	if (CanvasCenter.Y != 0.5f)
	{
		Vertex.Y -= CanvasInfo.Size.Y * (CanvasCenter.Y - 0.5f);
	}
	
	Vertex.Y = CanvasInfo.Size.Y - Vertex.Y;

	return Vertex;
}

bool ULive2DMocModel::InitializeMoc(uint8* Source)
{
	Moc = csmReviveMocInPlace(Source, MocSourceSize);
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
		InitializePartOpacities();
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
		ParameterValues.Add(ParameterId, ModelParameterValues[ParameterIndex]);
		ParameterDefaultValues.Add(ParameterId, ModelParameterDefaultValues[ParameterIndex]);
		ParameterMinimumValues.Add(ParameterId, ModelParameterMinimumValues[ParameterIndex]);
		ParameterMaximumValues.Add(ParameterId, ModelParameterMaximumValues[ParameterIndex]);
	}
}

void ULive2DMocModel::InitializePartOpacities()
{
	const int32 PartCount = csmGetPartCount(Model);
	const char** ModelPartIds = csmGetPartIds(Model);
	float* ModelPartOpacities = csmGetPartOpacities(Model);

	for(int32 PartIndex = 0; PartIndex < PartCount; PartIndex++)
	{
		const FString PartId = ModelPartIds[PartIndex];
		PartOpacities.Add(PartId, ModelPartOpacities[PartIndex]);
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

	Drawables.Sort([](const FLive2DModelDrawable& l, const FLive2DModelDrawable& r)
	{
		return (l.RenderOrder < r.RenderOrder);
	});
}
