// Fill out your copyright notice in the Description page of Project Settings.


#include "Live2DMocModel.h"

#include "CanvasItem.h"
#include "Live2DLogCategory.h"
#include "HAL/PlatformFilemanager.h"
#include "Live2DCubismCore.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Live2DModelPhysics.h"

ULive2DMocModel::ULive2DMocModel()
	: Super()
{
	Physics = CreateDefaultSubobject<ULive2DModelPhysics>(TEXT("Physics"));
}

UWorld* ULive2DMocModel::GetWorld() const
{
	// This implementation is needed so the blueprint can access worldcontext methods from blueprintFunctionLibraries. The check for for the defaultObject is there because in the editor the object doesn't have an outer.
	// A more detailed explanation is here: https://answers.unrealengine.com/questions/468741/how-to-make-a-blueprint-derived-from-a-uobject-cla.html
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		return GetOuter()->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

bool ULive2DMocModel::Init(const FString& FileName, const TArray<FModel3GroupData>& InGroups)
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

	Groups = InGroups;	

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

float ULive2DMocModel::GetModelWidth() const
{
	return GetModelSize().X;
	
}

float ULive2DMocModel::GetModelHeight() const
{
	return GetModelSize().Y;
}

FVector2D ULive2DMocModel::GetModelSize() const
{
	auto CanvasInfo = GetModelCanvasInfoInternal();
	FVector2D CanvasCenter(CanvasInfo.PivotOrigin.X/CanvasInfo.PixelsPerUnit,CanvasInfo.PivotOrigin.Y/CanvasInfo.PixelsPerUnit);
	FVector2D CanvasDimensions(CanvasInfo.Size.X/CanvasInfo.PixelsPerUnit,CanvasInfo.Size.Y/CanvasInfo.PixelsPerUnit);

	return CanvasInfo.Size / CanvasDimensions;
}

ULive2DModelPhysics* ULive2DMocModel::GetPhysicsSystem()
{
	return Physics;
}

void ULive2DMocModel::UpdateDrawables()
{
	csmResetDrawableDynamicFlags(Model);
	csmUpdateModel(Model);
	
	int DrawableCount = csmGetDrawableCount(Model);
	const int* VertexCounts = csmGetDrawableVertexCounts(Model);
	const csmVector2** VertexPositions = csmGetDrawableVertexPositions(Model);
	const csmVector2** VertexUvs = csmGetDrawableVertexUvs(Model);
	const int* IndexCounts = csmGetDrawableIndexCounts(Model);
	const unsigned short** VertexIndices = csmGetDrawableIndices(Model);
	const float* Opacities = csmGetDrawableOpacities(Model);
	const int* DrawOrders = csmGetDrawableDrawOrders(Model);
	const int* RenderOrders = csmGetDrawableRenderOrders(Model);
	const csmFlags* DynamicFlags = csmGetDrawableDynamicFlags(Model);

	for (int32 ModelDrawableIndex = 0; ModelDrawableIndex < DrawableCount; ModelDrawableIndex++)
	{
		FLive2DModelDrawable& Drawable = Drawables[ModelDrawableIndex];

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

	UpdateRenderTarget();
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

float ULive2DMocModel::GetMinimumParameterValue(const FString& ParameterName)
{
	auto* MinimumParameterValue = ParameterMinimumValues.Find(ParameterName);

	if (!MinimumParameterValue)
	{
		UE_LOG(LogLive2D, Error, TEXT("ULive2DMocModel::GetMinimumParameterValue: Parameter %s doesn't exist on Live 2D Model!"), *ParameterName);
		return 0.f;
	}

	return *MinimumParameterValue;
}

float ULive2DMocModel::GetMaximumParameterValue(const FString& ParameterName)
{
	auto* MaximumParameterValue = ParameterMaximumValues.Find(ParameterName);

	if (!MaximumParameterValue)
	{
		UE_LOG(LogLive2D, Error, TEXT("ULive2DMocModel::GetMaximumParameterValue: Parameter %s doesn't exist on Live 2D Model!"), *ParameterName);
		return 0.f;
	}

	return *MaximumParameterValue;
}

float ULive2DMocModel::GetDefaultParameterValue(const FString& ParameterName)
{
	auto* DefaultParameterValue = ParameterDefaultValues.Find(ParameterName);

	if (!DefaultParameterValue)
	{
		UE_LOG(LogLive2D, Error, TEXT("ULive2DMocModel::GetDefaultParameterValue: Parameter %s doesn't exist on Live 2D Model!"), *ParameterName);
		return 0.f;
	}

	return *DefaultParameterValue;
}

void ULive2DMocModel::SetParameterValue(const FString& ParameterName, const float Value, const bool bUpdateDrawables)
{
	TArray<FString> AffectedIds;
	if (GetAffectedParameterIdsByGroupName(ParameterName, TEXT("Parameter"), AffectedIds))
	{
		for (const auto& AffectedId: AffectedIds)
		{
			SetParameterValueInternal(AffectedId, Value, bUpdateDrawables);
		}
		return;
	}
	
	SetParameterValueInternal(ParameterName, Value, bUpdateDrawables);
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
	TArray<FString> AffectedIds;
	if (GetAffectedParameterIdsByGroupName(ParameterName, TEXT("PartOpacity"), AffectedIds))
	{
		for (const auto& AffectedId: AffectedIds)
		{
			SetPartOpacityValueInternal(AffectedId, Value, bUpdateDrawables);
		}
		return;
	}
	
	SetPartOpacityValueInternal(ParameterName, Value, bUpdateDrawables);
}

FSlateBrush& ULive2DMocModel::GetTexture2DRenderTarget()
{
	if (!RenderTarget2D)
	{
		SetupRenderTarget();
		UpdateRenderTarget();
	}
	
	return RenderTargetBrush; 
}

void ULive2DMocModel::StartTicking(const float TickRate)
{
	UWorld* World =
#if WITH_EDITOR
	GWorld;
#else
	GetWorld();
#endif

	World->GetTimerManager().SetTimer(TickHandle,  FTimerDelegate::CreateUObject(this, &ULive2DMocModel::OnTick, TickRate), TickRate, true);
}

void ULive2DMocModel::StopTicking()
{
	UWorld* World =
#if WITH_EDITOR
	GWorld;
#else
	GetWorld();
#endif
	
	World->GetTimerManager().ClearTimer(TickHandle);
}

void ULive2DMocModel::OnTick(const float DeltaTime)
{
	OnModelTick.Broadcast(DeltaTime);

	Physics->Evaluate(DeltaTime);
	
	UpdateDrawables();
}

FLive2DModelCanvasInfo ULive2DMocModel::GetModelCanvasInfoInternal() const
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

bool ULive2DMocModel::GetAffectedParameterIdsByGroupName(const FString& GroupName, const FString& TargetName, TArray<FString>& AffectedIds )
{
	AffectedIds.Empty();
	for (const auto& Group: Groups)
	{
		if (Group.Name == GroupName && Group.Target == TargetName)
		{
			AffectedIds = Group.Ids;
			return true;
		}
	}

	return false;
}

void ULive2DMocModel::SetParameterValueInternal(const FString& ParameterName, const float Value, const bool bUpdateDrawables)
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

	if (ParameterName == TEXT("ParamArmL"))
	{
		UE_LOG(LogLive2D, Warning, TEXT("ParamArmL: %f"), *ParameterValue);
	}
	
	
	if (bUpdateDrawables)
	{
		UpdateDrawables();
	}
}

void ULive2DMocModel::SetPartOpacityValueInternal(const FString& ParameterName, const float Value, const bool bUpdateDrawables)
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

void ULive2DMocModel::SetupRenderTarget()
{
	if (RenderTarget2D)
	{
		return;
	}
	
	const FVector2D ModelSize = GetModelSize();
	RenderTarget2D = NewObject<UTextureRenderTarget2D>(this);
	check(RenderTarget2D);
	RenderTarget2D->TargetGamma = 1.f;
	RenderTarget2D->RenderTargetFormat = RTF_RGBA8;
	RenderTarget2D->ClearColor = FLinearColor::Transparent;
	RenderTarget2D->bAutoGenerateMips = false;
	if (auto* Texture = Textures[0])
	{
		RenderTarget2D->InitCustomFormat(ModelSize.X, ModelSize.Y, Texture->GetPixelFormat(), Texture->bUseLegacyGamma);
	}
	else
	{
		RenderTarget2D->InitAutoFormat(ModelSize.X, ModelSize.Y);
	}
	RenderTarget2D->UpdateResourceImmediate(true);
	
	RenderTargetBrush.SetResourceObject(RenderTarget2D);
	RenderTargetBrush.ImageSize = ModelSize;
	RenderTargetBrush.DrawAs = ESlateBrushDrawType::Image;
	RenderTargetBrush.TintColor = FLinearColor::White;
}

void ULive2DMocModel::UpdateRenderTarget()
{
	auto CanvasInfo = GetModelCanvasInfoInternal();
	UWorld* World =
#if WITH_EDITOR
	GWorld;
#else
	GetWorld();
#endif

	UCanvas* Canvas;

	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::ClearRenderTarget2D(World, RenderTarget2D, FLinearColor::Transparent);
	FVector2D Size;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(World, RenderTarget2D, Canvas, Size, Context);

	for (const auto& Drawable: Drawables)
	{
		ProcessMasksOfDrawable(Drawable, Canvas, CanvasInfo);
		
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
			Triangle.V0_Color.A = Drawable.Opacity;
			Triangle.V1_Color = FLinearColor::White;
			Triangle.V1_Color.A = Drawable.Opacity;
			Triangle.V2_Color = FLinearColor::White;
			Triangle.V2_Color.A = Drawable.Opacity;

			TriangleList.Add(Triangle);
		}
		
		FCanvasTriangleItem TriangleItem(TriangleList, Textures[Drawable.TextureIndex]->GetResource());

		switch (Drawable.BlendMode)
		{
		case ELive2dModelBlendMode::ADDITIVE_BLENDING:
			TriangleItem.BlendMode = SE_BLEND_Additive;
			break;
		case ELive2dModelBlendMode::MULTIPLICATIVE_BLENDING:
			TriangleItem.BlendMode = SE_BLEND_Modulate;
			break;
		case ELive2dModelBlendMode::NORMAL_BLENDING:
		default:
			TriangleItem.BlendMode = SE_BLEND_Masked;
			break;
		}

		Canvas->DrawItem(TriangleItem);
	}	
}

void ULive2DMocModel::ProcessMasksOfDrawable(const FLive2DModelDrawable& Drawable, UCanvas* Canvas, const FLive2DModelCanvasInfo& CanvasInfo)
{
	if (Drawable.Masks.Num() == 0)
	{
		return;
	}

	uint32 Result = (uint32)SE_BLEND_RGBA_MASK_START;
	Result += Drawable.Masks.Num() == 1 ? (1 << 0) : 0; // R
	Result += Drawable.Masks.Num() == 1 ? (1 << 1) : 0; // G
	Result += Drawable.Masks.Num() == 1 ? (1 << 2) : 0; // B
	Result += (1 << 3);									// A
	ESimpleElementBlendMode BlendMode = static_cast<ESimpleElementBlendMode>(Result);

	for (const int32 MaskIndex: Drawable.Masks)
	{
		if (MaskIndex == -1)
		{
			continue;
		}
		
		const auto& MaskDrawable = Drawables[MaskIndex];
		TArray<FCanvasUVTri> TriangleList;

		for (int32 i = 0; i < MaskDrawable.VertexIndices.Num(); i += 3)
		{
			const int32 VertexIndex0 = MaskDrawable.VertexIndices[i];
			const int32 VertexIndex1 = MaskDrawable.VertexIndices[i+1];
			const int32 VertexIndex2 = MaskDrawable.VertexIndices[i+2];
			
			FCanvasUVTri Triangle;
			Triangle.V0_Pos = ProcessVertex(MaskDrawable.VertexPositions[VertexIndex0], CanvasInfo);
			Triangle.V1_Pos = ProcessVertex(MaskDrawable.VertexPositions[VertexIndex1], CanvasInfo);
			Triangle.V2_Pos = ProcessVertex(MaskDrawable.VertexPositions[VertexIndex2], CanvasInfo);
			Triangle.V0_UV = MaskDrawable.VertexUVs[VertexIndex0];
			Triangle.V0_UV.Y = 1 - Triangle.V0_UV.Y;
			Triangle.V1_UV = MaskDrawable.VertexUVs[VertexIndex1];
			Triangle.V1_UV.Y = 1 - Triangle.V1_UV.Y;
			Triangle.V2_UV = MaskDrawable.VertexUVs[VertexIndex2];
			Triangle.V2_UV.Y = 1 - Triangle.V2_UV.Y;
			Triangle.V0_Color = FLinearColor::White;
			Triangle.V1_Color = FLinearColor::White;
			Triangle.V2_Color = FLinearColor::White;

			TriangleList.Add(Triangle);
		}
		
		FCanvasTriangleItem TriangleItem(TriangleList, Textures[MaskDrawable.TextureIndex]->GetResource());
		
		TriangleItem.BlendMode = BlendMode;

		Canvas->DrawItem(TriangleItem);
	}
}

FVector2D ULive2DMocModel::ProcessVertex(FVector2D Vertex, const FLive2DModelCanvasInfo& CanvasInfo)
{
	FVector2D CanvasCenter(CanvasInfo.PivotOrigin.X/CanvasInfo.PixelsPerUnit,CanvasInfo.PivotOrigin.Y/CanvasInfo.PixelsPerUnit);
	FVector2D CanvasDimensions(CanvasInfo.Size.X/CanvasInfo.PixelsPerUnit,CanvasInfo.Size.Y/CanvasInfo.PixelsPerUnit);
	Vertex += (FVector2D(0.5f));
	
	if (CanvasCenter.X != 0.5f)
	{
		Vertex.X += (CanvasCenter.X - 0.5f);
	}
	
	if (CanvasCenter.Y != 0.5f)
	{
		Vertex.Y += (CanvasCenter.Y - 0.5f);
	}
	
	Vertex = FMath::Abs(Vertex);
	
	Vertex /= CanvasDimensions;
	Vertex.Y = 1.f - Vertex.Y;
	Vertex *= CanvasInfo.Size / CanvasDimensions;

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
