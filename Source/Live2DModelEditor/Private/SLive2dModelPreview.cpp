// Fill out your copyright notice in the Description page of Project Settings.


#include "SLive2dModelPreview.h"

#include "CanvasItem.h"
#include "SlateOptMacros.h"

/**
* Simple representation of the backbuffer that the preview canvas renders to
* This class may only be accessed from the render thread
*/
class FSlateMaterialPreviewRenderTarget : public FRenderTarget
{
public:
	/** FRenderTarget interface */
	virtual FIntPoint GetSizeXY() const
	{
		return ClippingRect.Size();
	}

	/** Sets the texture that this target renders to */
	void SetRenderTargetTexture( FTexture2DRHIRef& InRHIRef )
	{
		RenderTargetTextureRHI = InRHIRef;
	}

	/** Clears the render target texture */
	void ClearRenderTargetTexture()
	{
		RenderTargetTextureRHI.SafeRelease();
	}

	/** Sets the viewport rect for the render target */
	void SetViewRect( const FIntRect& InViewRect ) 
	{ 
		ViewRect = InViewRect;
	}

	/** Gets the viewport rect for the render target */
	const FIntRect& GetViewRect() const 
	{
		return ViewRect;
	}

	/** Sets the clipping rect for the render target */
	void SetClippingRect( const FIntRect& InClippingRect ) 
	{ 
		ClippingRect = InClippingRect;
	}

	/** Gets the clipping rect for the render target */
	const FIntRect& GetClippingRect() const 
	{
		return ClippingRect;
	}

private:
	FIntRect ViewRect;
	FIntRect ClippingRect;
};

class FLive2DModelElement : public ICustomSlateElement
{
public:
	FLive2DModelElement()
	: RenderTarget(new FSlateMaterialPreviewRenderTarget)
	{
	}
	
	~FLive2DModelElement()
	{
		delete RenderTarget;
	}

	/**
	 * Sets up the canvas for rendering
	 *
	 * @param	InCanvasRect			Size of the canvas tile
	 * @param	InClippingRect			How to clip the canvas tile
	 * @param	InGraphNode				The graph node for the material preview
	 * @param	bInIsRealtime			Whether preview is using realtime values
	 *
	 * @return	Whether there is anything to render
	 */
	bool BeginRenderingCanvas(const FIntRect& InCanvasRect, const FIntRect& InClippingRect, const FLive2DModelRenderData& RenderData)
	{
		if(InCanvasRect.Size().X > 0 && InCanvasRect.Size().Y > 0 && InClippingRect.Size().X > 0 && InClippingRect.Size().Y > 0)
		{
			/**
			 * Struct to contain all info that needs to be passed to the render thread
			 */
			struct FPreviewRenderInfo
			{
				/** Size of the Canvas tile */
				FIntRect CanvasRect;
				/** How to clip the canvas tile */
				FIntRect ClippingRect;

				FLive2DModelRenderData RenderData;
			};

			FPreviewRenderInfo RenderInfo;
			RenderInfo.CanvasRect = InCanvasRect;
			RenderInfo.ClippingRect = InClippingRect;
			RenderInfo.RenderData = RenderData;

			FLive2DModelElement* Live2DModelElement = this;
			ENQUEUE_RENDER_COMMAND(BeginRenderingPreviewCanvas)(
				[Live2DModelElement, RenderInfo](FRHICommandListImmediate& RHICmdList)
				{
					Live2DModelElement->RenderTarget->SetViewRect(RenderInfo.CanvasRect);
					Live2DModelElement->RenderTarget->SetClippingRect(RenderInfo.ClippingRect);
					Live2DModelElement->RenderData  = RenderInfo.RenderData;
				}
			);
			return true;
		}

		return false;
	}

private:
	/**
	 * ICustomSlateElement interface 
	 */
	virtual void DrawRenderThread(FRHICommandListImmediate& RHICmdList, const void* InWindowBackBuffer) override
	{
		if(RenderData.Texture)
		{
			RenderTarget->SetRenderTargetTexture(*(FTexture2DRHIRef*)InWindowBackBuffer);
			{
				FCanvas Canvas(RenderTarget, NULL, 0, 0, 0, GMaxRHIFeatureLevel);
				{
					Canvas.SetAllowedModes(0);
					Canvas.SetRenderTargetRect(RenderTarget->GetViewRect());
					Canvas.SetRenderTargetScissorRect(RenderTarget->GetClippingRect());

					FCanvasTileItem TileItem(FVector2D::ZeroVector, RenderData.Texture->Resource, RenderTarget->GetSizeXY(), FLinearColor::White);
					Canvas.DrawItem(TileItem);
				}
				Canvas.Flush_RenderThread(RHICmdList, true);
			}
			RenderTarget->ClearRenderTargetTexture();
			RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
		}
	}

private:
	/** Render target that the canvas renders to */
	FSlateMaterialPreviewRenderTarget* RenderTarget;

	FLive2DModelRenderData RenderData;
};

typedef TSharedPtr<FLive2DModelElement, ESPMode::ThreadSafe> FThreadSafeFLive2DModelElementPtr;

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLive2dModelPreview::Construct(const FArguments& InArgs, ULive2DMocModel* InLive2DMocModel)
{
	Live2DMocModel = InLive2DMocModel;

	TArray<FLive2DModelDrawable> Drawables = Live2DMocModel->Drawables;
	FLive2DModelCanvasInfo CanvasInfo = Live2DMocModel->GetModelCanvasInfo();
	FVector2D CanvasCenter(CanvasInfo.PivotOrigin.X/CanvasInfo.PixelsPerUnit,CanvasInfo.PivotOrigin.Y/CanvasInfo.PixelsPerUnit);
	FVector2D CanvasDimensions(CanvasInfo.Size.X/CanvasInfo.PixelsPerUnit,CanvasInfo.Size.Y/CanvasInfo.PixelsPerUnit);
	
	RenderData.Empty();

	for (const auto& Drawable: Drawables)
	{
		if (!Drawable.IsVisible())
		{
			continue;
		}
		
		FLive2DModelRenderData ModelRenderData;

		for (int32 i = 0; i < Drawable.VertexPositions.Num(); i++)
		{
			FSlateVertex Vertex = FSlateVertex::
				Make<ESlateVertexRounding::Disabled>(FSlateRenderTransform(),
				                                     Drawable.VertexPositions[i],
				                                     Drawable.VertexUVs[i],
				                                     FColor::White);


			//Vertex.Position.X = (Vertex.Position.X/CanvasInfo.PixelsPerUnit) - (CanvasCenter.X * CanvasDimensions.X);
			//Vertex.Position.Y = (CanvasCenter.Y * CanvasDimensions.Y) - (Vertex.Position.Y/CanvasInfo.PixelsPerUnit);
			Vertex.Position *= CanvasInfo.Size;
			Vertex.Position.X += (CanvasInfo.Size.X * CanvasCenter.X);
			Vertex.Position.Y += (CanvasInfo.Size.Y * CanvasCenter.Y);
			Vertex.Position.Y = CanvasInfo.Size.Y - Vertex.Position.Y;

			Vertex.TexCoords[1] = 1 - Vertex.TexCoords[1];

			Vertex.Color.A = Drawable.Opacity*255.f;
			
			ModelRenderData.Vertices.Add(Vertex);
		}

		for (int i = 0; i < Drawable.VertexIndices.Num(); i++)
		{
			ModelRenderData.Indices.Add(Drawable.VertexIndices[i]);
		}

		UTexture2D* Texture = Live2DMocModel->Textures[Drawable.TextureIndex];
		ModelRenderData.Texture = Texture;
		ModelRenderData.TextureBrush.SetResourceObject(Texture);
		ModelRenderData.TextureBrush.ImageSize.X = Texture->GetSizeX();
		ModelRenderData.TextureBrush.ImageSize.Y = Texture->GetSizeY();

		ModelRenderData.Layer = Drawable.RenderOrder;
		
		RenderData.Add(ModelRenderData);
	}
}

int32 SLive2dModelPreview::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(), FEditorStyle::GetBrush("WhiteBrush"));

	int32 NewLayerId = LayerId;
	
	for (const auto& RenderDatum: RenderData)
	{
		const int32 CurrentLayerId = LayerId + RenderDatum.Layer;
		// FSlateDrawElement::MakeCustomVerts(OutDrawElements, CurrentLayerId, RenderDatum.TextureBrush.GetRenderingResource(), RenderDatum.Vertices, RenderDatum.Indices, nullptr, 0, 0);
		// NewLayerId = FMath::Max(NewLayerId, CurrentLayerId);
		if ()
	}
	
	return NewLayerId;
}

FVector2D SLive2dModelPreview::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	if (!Live2DMocModel)
	{
		return FVector2D::ZeroVector;
	}

	FLive2DModelCanvasInfo CanvasInfo = Live2DMocModel->GetModelCanvasInfo();

	return CanvasInfo.Size;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
