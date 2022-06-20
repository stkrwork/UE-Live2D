// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Live2DMocModel.h"
#include "Widgets/SWidget.h"

struct FLive2DModelRenderData
{
	TArray<FSlateVertex> Vertices;
	TArray<SlateIndex> Indices;
	FSlateBrush TextureBrush;
	int32 Layer;
	UTexture2D* Texture;
};

/**
 * 
 */
class LIVE2D_API SLive2dModelImage : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SLive2dModelImage)
		{
		}

	SLATE_END_ARGS()

	virtual ~SLive2dModelImage() override;
	
	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, ULive2DMocModel* InLive2DMocModel);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

protected:

	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void UpdateRenderData();
	
	ULive2DMocModel* Live2DMocModel = nullptr;

	TArray<FLive2DModelRenderData> RenderData;
	float ScaleFactor = 1.f;
};
