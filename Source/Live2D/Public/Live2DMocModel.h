// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Live2DCubismCore.h"
#include "Live2DStructs.h"
#include "UObject/Object.h"
#include "Engine/Texture2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Live2DMocModel.generated.h"

class ULive2DModelPhysics;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LIVE2D_API ULive2DMocModel : public UObject
{
	GENERATED_BODY()

public:
	ULive2DMocModel();
	virtual UWorld* GetWorld() const override;
	
	bool Init(const FString& FileName, const TArray<FModel3GroupData>& InGroups = {});

	virtual void BeginDestroy() override;

	virtual void Serialize(FArchive& Ar) override;

	float GetModelWidth() const;
	float GetModelHeight() const;
	FVector2D GetModelSize() const;

	ULive2DModelPhysics* GetPhysicsSystem();;

	void UpdateDrawables();

	float GetParameterValue(const FString& ParameterName);
	float GetMinimumParameterValue(const FString& ParameterName);
	float GetMaximumParameterValue(const FString& ParameterName);
	float GetDefaultParameterValue(const FString& ParameterName);
	void SetParameterValue(const FString& ParameterName, const float Value, const bool bUpdateDrawables = false);

	float GetPartOpacityValue(const FString& ParameterName);
	void SetPartOpacityValue(const FString& ParameterName, const float Value, const bool bUpdateDrawables = false);

	FSlateBrush& GetTexture2DRenderTarget();

	bool IsTicking() const { return TickHandle.IsValid(); }
	void StartTicking(const float TickRate);
	void StopTicking();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModelTick, const float, DeltaTime);

	UPROPERTY(BlueprintAssignable)
	FOnModelTick OnModelTick;

	DECLARE_MULTICAST_DELEGATE(FOnDrawablesUpdated);

	FOnDrawablesUpdated OnDrawablesUpdated;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	TArray<FLive2DModelDrawable> Drawables;
	TArray<FLive2DModelDrawable> UnSortedDrawables;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UTexture2D*> Textures;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FModel3GroupData> Groups;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	TMap<FString, float> PartOpacities;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	TMap<FString, float> ParameterValues;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	TMap<FString, float> ParameterDefaultValues;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	TMap<FString, float> ParameterMaximumValues;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	TMap<FString, float> ParameterMinimumValues;

	UPROPERTY(Transient)
	UTextureRenderTarget2D* RenderTarget2D = nullptr;

	UPROPERTY(Transient)
	TMap<FString, UTextureRenderTarget2D*> MaskingRenderTargets;

	UPROPERTY(Transient)
	FSlateBrush RenderTargetBrush;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ULive2DModelPhysics* Physics;
private:

	void OnTick(const float DeltaTime);

	UPROPERTY()
	FTimerHandle TickHandle;

	FLive2DModelCanvasInfo GetModelCanvasInfoInternal() const;
	bool GetAffectedParameterIdsByGroupName(const FString& GroupName, const FString& TargetName, TArray<FString>& AffectedIds);
	void SetParameterValueInternal(const FString& ParameterName, const float Value, const bool bUpdateDrawables = false);
	void SetPartOpacityValueInternal(const FString& ParameterName, const float Value, const bool bUpdateDrawables = false);
	void SetupRenderTarget();
	void UpdateRenderTarget();
	void ProcessMaskedDrawable(const FLive2DModelDrawable& Drawable, UCanvas*& Canvas, const FLive2DModelCanvasInfo& CanvasInfo, FDrawToRenderTargetContext& Context);
	void ProcessNonMaskedDrawable(const FLive2DModelDrawable& Drawable, UCanvas* Canvas, const FLive2DModelCanvasInfo& CanvasInfo);
	FVector2D ProcessVertex(FVector2D Vertex, const FLive2DModelCanvasInfo& CanvasInfo);
	bool InitializeMoc(uint8* Source);
	bool InitializeModel();
	void InitializeParameterList();
	void InitializePartOpacities();
	void InitializeDrawables();

	UPROPERTY()
	int32 MocSourceSize;

	uint8* MocSource;
	csmMoc* Moc;
	csmModel* Model;
};
