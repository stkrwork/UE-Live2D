#include "Live2DBatchedElements.h"
#include "GlobalShader.h"
#include "Live2DLogCategory.h"
#include "Live2DSimpleElementShaders.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterMacros.h"
#include "Engine/TextureRenderTarget2D.h"

float GBatchedElementAlphaRefVal = 128.f;

class FLive2DMaskedShader : public FGlobalShader
{
public:
	DECLARE_EXPORTED_SHADER_TYPE(FLive2DMaskedShader, Global, LIVE2D_API);
	SHADER_USE_PARAMETER_STRUCT(FLive2DMaskedShader, FGlobalShader);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_TEXTURE(Texture2D, InMainTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InMainTextureSampler)
		SHADER_PARAMETER_TEXTURE(Texture2D, InMaskTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InMaskTextureSampler)
		SHADER_PARAMETER(float, InGamma)
		SHADER_PARAMETER(FVector2D, InMaskSize)
		SHADER_PARAMETER(float, InClipRef)
	END_SHADER_PARAMETER_STRUCT()
};

template<bool InvertedMask>
class FLive2DMaskShader : public FGlobalShader
{
public:
	DECLARE_EXPORTED_SHADER_TYPE(FLive2DMaskShader, Global, LIVE2D_API);
	SHADER_USE_PARAMETER_STRUCT(FLive2DMaskShader, FGlobalShader);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("LIVE_2D_INVERTED_MASK"), InvertedMask);
	}

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_TEXTURE(Texture2D, InMaskTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InMaskTextureSampler)
		SHADER_PARAMETER(float, InGamma)
		SHADER_PARAMETER(float, InClipRef)
	END_SHADER_PARAMETER_STRUCT()
};


IMPLEMENT_GLOBAL_SHADER(FLive2DMaskedShader, "/Plugin/UELive2D/Private/Live2DMaskedBatchedElements.usf", "MainPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FLive2DMaskShader<true>, "/Plugin/UELive2D/Private/Live2DMaskBatchedElements.usf", "MainPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FLive2DMaskShader<false>, "/Plugin/UELive2D/Private/Live2DMaskBatchedElements.usf", "MainPS", SF_Pixel);


void FLive2DMaskedBatchedElements::BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture)
{
	// TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(InFeatureLevel));
	// 		
	// GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSimpleElementVertexDeclaration.VertexDeclarationRHI;
	// GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	// GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_Zero, BF_One>::GetRHI();
	//
	// if (Texture->bSRGB)
	// {
	// 	TShaderMapRef<FSimpleElementMaskedGammaPS_SRGB> MaskedPixelShader(GetGlobalShaderMap(InFeatureLevel));
	// 	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = MaskedPixelShader.GetPixelShader();
	// 	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
	//
	// 	MaskedPixelShader->SetEditorCompositingParameters(RHICmdList, nullptr);
	// 	MaskedPixelShader->SetParameters(RHICmdList, Texture, InGamma, GBatchedElementAlphaRefVal / 255.0f, SE_BLEND_Masked);
	// }
	// else
	// {
	// 	TShaderMapRef<FSimpleElementMaskedGammaPS_Linear> MaskedPixelShader(GetGlobalShaderMap(InFeatureLevel));
	// 	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = MaskedPixelShader.GetPixelShader();
	//
	// 	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
	//
	// 	MaskedPixelShader->SetEditorCompositingParameters(RHICmdList, nullptr);
	// 	MaskedPixelShader->SetParameters(RHICmdList, Texture, InGamma, GBatchedElementAlphaRefVal / 255.0f, SE_BLEND_Masked);
	// }
	//
	// VertexShader->SetParameters(RHICmdList, InTransform);
	
	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(InFeatureLevel));
	TShaderMapRef<FLive2DMaskedShader> PixelShader(GetGlobalShaderMap(InFeatureLevel));
	
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSimpleElementVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_One>::GetRHI();
	
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, EApplyRendertargetOption::ForceApply);
	
	VertexShader->SetParameters(RHICmdList, InTransform);
	
	FLive2DMaskedShader::FParameters PassParameters;
	PassParameters.InMainTexture = Texture2D->GetResource()->TextureRHI;
	PassParameters.InMainTextureSampler = Texture2D->GetResource()->SamplerStateRHI;
	PassParameters.InMaskTexture = MaskRenderTarget->GetResource()->TextureRHI;
	PassParameters.InMaskTextureSampler = MaskRenderTarget->GetResource()->SamplerStateRHI;
	PassParameters.InMaskSize = FVector2D(MaskRenderTarget->SizeX, MaskRenderTarget->SizeY);
	PassParameters.InGamma = InGamma;
	PassParameters.InClipRef = GBatchedElementAlphaRefVal / 255.0f;
	SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters);
}

void FLive2DMaskBatchedElements::BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture)
{
	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(InFeatureLevel));
			
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSimpleElementVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();

	if (Texture->bSRGB)
	{
		TShaderMapRef<FSimpleElementMaskedGammaPS_SRGB> MaskedPixelShader(GetGlobalShaderMap(InFeatureLevel));
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = MaskedPixelShader.GetPixelShader();
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		MaskedPixelShader->SetEditorCompositingParameters(RHICmdList, nullptr);
		MaskedPixelShader->SetParameters(RHICmdList, Texture, InGamma, GBatchedElementAlphaRefVal / 255.0f, SE_BLEND_Masked);
	}
	else
	{
		TShaderMapRef<FSimpleElementMaskedGammaPS_Linear> MaskedPixelShader(GetGlobalShaderMap(InFeatureLevel));
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = MaskedPixelShader.GetPixelShader();

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		MaskedPixelShader->SetEditorCompositingParameters(RHICmdList, nullptr);
		MaskedPixelShader->SetParameters(RHICmdList, Texture, InGamma, GBatchedElementAlphaRefVal / 255.0f, SE_BLEND_Masked);
	}
	
	VertexShader->SetParameters(RHICmdList, InTransform);
	// FRHITexture* RHITexture = nullptr;
	// FRHISamplerState* RHISamplerState = nullptr;
	//
	// if ( Texture != nullptr )
	// {
	// 	RHITexture = Texture->TextureRHI;
	// 	RHISamplerState = Texture->SamplerStateRHI;
	// }
	// else
	// {
	// 	UE_LOG(LogLive2D, Fatal, TEXT("FLive2DMaskBatchedElements::BindShaders: Texture parameter null!"));
	// 	return;
	// }
	//
	// TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(InFeatureLevel));
	// TShaderMapRef<FLive2DMaskShader<false>> PixelShader(GetGlobalShaderMap(InFeatureLevel));
	//
	// GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSimpleElementVertexDeclaration.VertexDeclarationRHI;
	// GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	// GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	// GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	// GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	//
	// SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, EApplyRendertargetOption::ForceApply);
	//
	// VertexShader->SetParameters(RHICmdList, InTransform);
	//
	// FLive2DMaskShader<false>::FParameters PassParameters;
	// PassParameters.InMaskTexture = RHITexture;
	// PassParameters.InMaskTextureSampler = RHISamplerState;
	// PassParameters.InGamma = InGamma;
	// PassParameters.InClipRef = GBatchedElementAlphaRefVal / 255.0f;
	// SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters);
}

void FLive2DInvertedMaskBatchedElements::BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture)
{
	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(InFeatureLevel));
			
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSimpleElementVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();

	if (Texture->bSRGB)
	{
		TShaderMapRef<FSimpleElementMaskedGammaPS_SRGB> MaskedPixelShader(GetGlobalShaderMap(InFeatureLevel));
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = MaskedPixelShader.GetPixelShader();
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		MaskedPixelShader->SetEditorCompositingParameters(RHICmdList, nullptr);
		MaskedPixelShader->SetParameters(RHICmdList, Texture, InGamma, GBatchedElementAlphaRefVal / 255.0f, SE_BLEND_Masked);
	}
	else
	{
		TShaderMapRef<FSimpleElementMaskedGammaPS_Linear> MaskedPixelShader(GetGlobalShaderMap(InFeatureLevel));
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = MaskedPixelShader.GetPixelShader();

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		MaskedPixelShader->SetEditorCompositingParameters(RHICmdList, nullptr);
		MaskedPixelShader->SetParameters(RHICmdList, Texture, InGamma, GBatchedElementAlphaRefVal / 255.0f, SE_BLEND_Masked);
	}
	
	VertexShader->SetParameters(RHICmdList, InTransform);
	// FRHITexture* RHITexture = nullptr;
	// FRHISamplerState* RHISamplerState = nullptr;
	//
	// if ( Texture != nullptr )
	// {
	// 	RHITexture = Texture->TextureRHI;
	// 	RHISamplerState = Texture->SamplerStateRHI;
	// }
	// else
	// {
	// 	UE_LOG(LogLive2D, Fatal, TEXT("FLive2DInvertedMaskBatchedElements::BindShaders: Texture parameter null!"));
	// 	return;
	// }
	//
	// TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(InFeatureLevel));
	// TShaderMapRef<FLive2DMaskShader<true>> PixelShader(GetGlobalShaderMap(InFeatureLevel));
	//
	// GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSimpleElementVertexDeclaration.VertexDeclarationRHI;
	// GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	// GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	// GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	// GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	//
	// SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, EApplyRendertargetOption::ForceApply);
	//
	// VertexShader->SetParameters(RHICmdList, InTransform);
	//
	// FLive2DMaskShader<true>::FParameters PassParameters;
	// PassParameters.InMaskTexture = RHITexture;
	// PassParameters.InMaskTextureSampler = RHISamplerState;
	// PassParameters.InGamma = InGamma;
	// PassParameters.InClipRef = GBatchedElementAlphaRefVal / 255.0f;
	// SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters);
}
