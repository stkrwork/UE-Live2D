// Fill out your copyright notice in the Description page of Project Settings.

#include "SLive2dModelImage.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLive2dModelImage::~SLive2dModelImage()
{
	Live2DMocModel->OnDrawablesUpdated.RemoveAll(this);
	Live2DMocModel = nullptr;
}

void SLive2dModelImage::Construct(const FArguments& InArgs, ULive2DMocModel* InLive2DMocModel)
{
	Live2DMocModel = InLive2DMocModel;

	UpdateRenderData();
	Live2DMocModel->OnDrawablesUpdated.AddRaw(this, &SLive2dModelImage::UpdateRenderData);
}

int32 SLive2dModelImage::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 NewLayerId = LayerId;

	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), RenderTargetBrush);
	
	// for (const auto& RenderDatum: RenderData)
	// {
	// 	const int32 CurrentLayerId = LayerId + RenderDatum.Layer;
	// 	FSlateDrawElement::MakeCustomVerts(OutDrawElements, CurrentLayerId, RenderDatum.TextureBrush.GetRenderingResource(), RenderDatum.Vertices, RenderDatum.Indices, nullptr, 0, 0);
	// 	NewLayerId = FMath::Max(NewLayerId, CurrentLayerId);
	// }
	
	return NewLayerId;
}

FVector2D SLive2dModelImage::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	if (!Live2DMocModel)
	{
		return FVector2D::ZeroVector;
	}

	FLive2DModelCanvasInfo CanvasInfo = Live2DMocModel->GetModelCanvasInfo();

	return CanvasInfo.Size;
}

FReply SLive2dModelImage::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	float Delta = MouseEvent.GetWheelDelta();
	ScaleFactor += Delta/10.f;
	UpdateRenderData();
	
	return FReply::Handled();
}

void SLive2dModelImage::UpdateRenderData()
{
	RenderTargetBrush = &Live2DMocModel->GetTexture2DRenderTarget();
	// TArray<FLive2DModelDrawable> Drawables = Live2DMocModel->Drawables;
	// FLive2DModelCanvasInfo CanvasInfo = Live2DMocModel->GetModelCanvasInfo();
	// FVector2D CanvasCenter(CanvasInfo.PivotOrigin.X/CanvasInfo.PixelsPerUnit,CanvasInfo.PivotOrigin.Y/CanvasInfo.PixelsPerUnit);
	// FVector2D CanvasDimensions(CanvasInfo.Size.X/CanvasInfo.PixelsPerUnit,CanvasInfo.Size.Y/CanvasInfo.PixelsPerUnit);
	//
	// RenderData.Empty();
	//
	// for (const auto& Drawable: Drawables)
	// {
	// 	if (!Drawable.IsVisible())
	// 	{
	// 		continue;
	// 	}
	// 	
	// 	FLive2DModelRenderData ModelRenderData;
	//
	// 	for (int32 i = 0; i < Drawable.VertexPositions.Num(); i++)
	// 	{
	// 		FSlateVertex Vertex = FSlateVertex::
	// 			Make<ESlateVertexRounding::Disabled>(FSlateRenderTransform(),
	// 			                                     Drawable.VertexPositions[i],
	// 			                                     Drawable.VertexUVs[i],
	// 			                                     FColor::White);
	//
	//
	// 		Vertex.Position *= CanvasInfo.Size;
	// 		Vertex.Position.X += (CanvasInfo.Size.X * CanvasCenter.X);
	// 		Vertex.Position.Y += (CanvasInfo.Size.Y * CanvasCenter.Y);
	//
	// 		// TODO move vertices to correct position so nothing gets rendered off-screen
	// 		if (CanvasCenter.X != 0.5f)
	// 		{
	// 			Vertex.Position.X += CanvasInfo.Size.X * (CanvasCenter.X - 0.5f);
	// 		}
	// 		
	// 		// TODO move vertices to correct position so nothing gets rendered off-screen
	// 		if (CanvasCenter.Y != 0.5f)
	// 		{
	// 			Vertex.Position.Y -= CanvasInfo.Size.Y * (CanvasCenter.Y - 0.5f);
	// 		}
	//
	// 		
	// 		Vertex.Position.Y = CanvasInfo.Size.Y - Vertex.Position.Y;
	//
	// 		Vertex.TexCoords[1] = 1 - Vertex.TexCoords[1];
	//
	// 		Vertex.Color.A = Drawable.Opacity*255.f;
	//
	// 		FSlateRenderTransform ScaledTransform(ScaleFactor);
	// 		Vertex.Position = TransformPoint(ScaledTransform, Vertex.Position);
	// 		
	// 		ModelRenderData.Vertices.Add(Vertex);
	// 	}
	//
	// 	for (int i = 0; i < Drawable.VertexIndices.Num(); i++)
	// 	{
	// 		ModelRenderData.Indices.Add(Drawable.VertexIndices[i]);
	// 	}
	//
	// 	UTexture2D* Texture = Live2DMocModel->Textures[Drawable.TextureIndex];
	// 	ModelRenderData.Texture = Texture;
	// 	ModelRenderData.TextureBrush.SetResourceObject(Texture);
	// 	ModelRenderData.TextureBrush.ImageSize.X = Texture ? Texture->GetSizeX() : 0;
	// 	ModelRenderData.TextureBrush.ImageSize.Y = Texture ? Texture->GetSizeY() : 0;
	//
	// 	ModelRenderData.Layer = Drawable.RenderOrder;
	// 	
	// 	RenderData.Add(ModelRenderData);
	// }

	Invalidate(EInvalidateWidgetReason::Paint);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
