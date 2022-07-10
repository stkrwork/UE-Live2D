#pragma once

#include "BatchedElements.h"
#include "CanvasItem.h"

class FLive2DMaskedBatchedElements : public FBatchedElementParameters
{
public:
	typedef TFunction<void(FRHITexture*&, FRHISamplerState*&)> FGetTextureAndSamplerDelegate;

	FLive2DMaskedBatchedElements(UTextureRenderTarget2D* InMaskRenderTarget, UTexture2D* InTexture2D, ESimpleElementBlendMode InBlendMode)
		: MaskRenderTarget(InMaskRenderTarget)
		, Texture2D(InTexture2D)
		, BlendMode(InBlendMode)
	{}

	/** Binds vertex and pixel shaders for this element */
	virtual void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture) override;

private:
	UTextureRenderTarget2D* MaskRenderTarget = nullptr;
	UTexture2D* Texture2D = nullptr;
	ESimpleElementBlendMode BlendMode = SE_BLEND_Masked;
};

class FLive2DMaskBatchedElements : public FBatchedElementParameters
{
public:
	typedef TFunction<void(FRHITexture*&, FRHISamplerState*&)> FGetTextureAndSamplerDelegate;

	FLive2DMaskBatchedElements(UTexture2D* InTexture2D)
		: Texture2D(InTexture2D)
	{}

	/** Binds vertex and pixel shaders for this element */
	virtual void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture) override;

private:
	UTexture2D* Texture2D = nullptr;
};

class FLive2DInvertedMaskBatchedElements : public FBatchedElementParameters
{
public:
	typedef TFunction<void(FRHITexture*&, FRHISamplerState*&)> FGetTextureAndSamplerDelegate;

	FLive2DInvertedMaskBatchedElements(UTexture2D* InTexture2D)
		: Texture2D(InTexture2D)
	{}

	/** Binds vertex and pixel shaders for this element */
	virtual void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture) override;

private:
	UTexture2D* Texture2D = nullptr;
};