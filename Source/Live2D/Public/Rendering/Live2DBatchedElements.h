#pragma once

#include "BatchedElements.h"

class FLive2DMaskedBatchedElements : public FBatchedElementParameters
{
public:
	typedef TFunction<void(FRHITexture*&, FRHISamplerState*&)> FGetTextureAndSamplerDelegate;

	FLive2DMaskedBatchedElements(UTextureRenderTarget2D* InMaskRenderTarget, UTexture2D* InTexture2D)
		: MaskRenderTarget(InMaskRenderTarget)
		, Texture2D(InTexture2D)
	{}

	/** Binds vertex and pixel shaders for this element */
	virtual void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture) override;

private:
	UTextureRenderTarget2D* MaskRenderTarget = nullptr;
	UTexture2D* Texture2D = nullptr;
};


class FLive2DMaskBatchedElements : public FBatchedElementParameters
{
public:
	typedef TFunction<void(FRHITexture*&, FRHISamplerState*&)> FGetTextureAndSamplerDelegate;

	FLive2DMaskBatchedElements(){}

	/** Binds vertex and pixel shaders for this element */
	virtual void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture) override;
};

class FLive2DInvertedMaskBatchedElements : public FBatchedElementParameters
{
public:
	typedef TFunction<void(FRHITexture*&, FRHISamplerState*&)> FGetTextureAndSamplerDelegate;

	FLive2DInvertedMaskBatchedElements(){}

	/** Binds vertex and pixel shaders for this element */
	virtual void BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture) override;
};