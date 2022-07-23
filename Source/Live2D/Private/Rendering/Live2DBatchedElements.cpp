#include "Live2DBatchedElements.h"
#include "GlobalShader.h"
#include "Live2DLogCategory.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterMacros.h"
#include "SimpleElementShaders.h"
#include "Engine/TextureRenderTarget2D.h"

namespace
{
	float GAlphaRefVal = 128.f;
}

class FLive2DNormalShader : public FGlobalShader
{
public:
	DECLARE_EXPORTED_SHADER_TYPE(FLive2DNormalShader, Global, LIVE2D_API);
	SHADER_USE_PARAMETER_STRUCT(FLive2DNormalShader, FGlobalShader);

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
		SHADER_PARAMETER(float, InGamma)
		SHADER_PARAMETER(float, InClipRef)
		SHADER_PARAMETER(float, RenderOpacity)
	END_SHADER_PARAMETER_STRUCT()
};

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
		SHADER_PARAMETER(float, RenderOpacity)
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
		SHADER_PARAMETER(float, RenderOpacity)
	END_SHADER_PARAMETER_STRUCT()
};


IMPLEMENT_GLOBAL_SHADER(FLive2DNormalShader, "/Plugin/UELive2D/Private/Live2DNormalBatchedElements.usf", "MainPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FLive2DMaskedShader, "/Plugin/UELive2D/Private/Live2DMaskedBatchedElements.usf", "MainPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FLive2DMaskShader<true>, "/Plugin/UELive2D/Private/Live2DMaskBatchedElements.usf", "MainPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FLive2DMaskShader<false>, "/Plugin/UELive2D/Private/Live2DMaskBatchedElements.usf", "MainPS", SF_Pixel);

void FLive2DNormalBatchedElements::BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture)
{	
	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(InFeatureLevel));
	TShaderMapRef<FLive2DNormalShader> PixelShader(GetGlobalShaderMap(InFeatureLevel));
	
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSimpleElementVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	switch (BlendMode)
	{
	case SE_BLEND_Additive:
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::GetRHI();
		break;
	case SE_BLEND_Modulate:
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_DestColor, BF_InverseDestColor, BO_Add, BF_DestAlpha, BF_InverseDestAlpha>::GetRHI();
		break;
	case SE_BLEND_Masked:
	default:
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha>::GetRHI();
		break;
	}
	
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, EApplyRendertargetOption::ForceApply);
	
	VertexShader->SetParameters(RHICmdList, InTransform);
	
	FLive2DNormalShader::FParameters PassParameters;
	PassParameters.InMainTexture = Texture2D->GetResource()->TextureRHI;
	PassParameters.InMainTextureSampler = Texture2D->GetResource()->SamplerStateRHI;
	PassParameters.InGamma = InGamma;
	PassParameters.InClipRef = GAlphaRefVal / 255.0f;
	PassParameters.RenderOpacity = RenderOpacity;
	SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters);
}

void FLive2DMaskedBatchedElements::BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture)
{	
	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(InFeatureLevel));
	TShaderMapRef<FLive2DMaskedShader> PixelShader(GetGlobalShaderMap(InFeatureLevel));
	
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSimpleElementVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	switch (BlendMode)
	{
	case SE_BLEND_Additive:
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_One, BF_One>::GetRHI();
		break;
	case SE_BLEND_Modulate:
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_DestColor, BF_Zero, BO_Add, BF_DestAlpha, BF_Zero>::GetRHI();
		break;
	case SE_BLEND_Masked:
	default:
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha>::GetRHI();
		break;
	}
	
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, EApplyRendertargetOption::ForceApply);
	
	VertexShader->SetParameters(RHICmdList, InTransform);
	
	FLive2DMaskedShader::FParameters PassParameters;
	PassParameters.InMainTexture = Texture2D->GetResource()->TextureRHI;
	PassParameters.InMainTextureSampler = Texture2D->GetResource()->SamplerStateRHI;
	PassParameters.InMaskTexture = MaskRenderTarget->GetResource()->TextureRHI;
	PassParameters.InMaskTextureSampler = MaskRenderTarget->GetResource()->SamplerStateRHI;
	PassParameters.InMaskSize = FVector2D(MaskRenderTarget->SizeX, MaskRenderTarget->SizeY);
	PassParameters.InGamma = InGamma;
	PassParameters.InClipRef = GAlphaRefVal / 255.0f;
	PassParameters.RenderOpacity = RenderOpacity;
	SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters);
}

void FLive2DMaskBatchedElements::BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture)
{
	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(InFeatureLevel));
	TShaderMapRef<FLive2DMaskShader<false>> PixelShader(GetGlobalShaderMap(InFeatureLevel));
	
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSimpleElementVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, EApplyRendertargetOption::ForceApply);
	
	VertexShader->SetParameters(RHICmdList, InTransform);
	
	FLive2DMaskShader<false>::FParameters PassParameters;
	PassParameters.InMaskTexture = Texture2D->GetResource()->TextureRHI;
	PassParameters.InMaskTextureSampler = Texture2D->GetResource()->SamplerStateRHI;
	PassParameters.InGamma = InGamma;
	PassParameters.InClipRef = GAlphaRefVal / 255.0f;
	PassParameters.RenderOpacity = RenderOpacity;
	SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters);
}

void FLive2DInvertedMaskBatchedElements::BindShaders(FRHICommandList& RHICmdList, FGraphicsPipelineStateInitializer& GraphicsPSOInit, ERHIFeatureLevel::Type InFeatureLevel, const FMatrix& InTransform, const float InGamma, const FMatrix& ColorWeights, const FTexture* Texture)
{	
	TShaderMapRef<FSimpleElementVS> VertexShader(GetGlobalShaderMap(InFeatureLevel));
	TShaderMapRef<FLive2DMaskShader<true>> PixelShader(GetGlobalShaderMap(InFeatureLevel));
	
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSimpleElementVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, EApplyRendertargetOption::ForceApply);
	
	VertexShader->SetParameters(RHICmdList, InTransform);
	
	FLive2DMaskShader<true>::FParameters PassParameters;
	PassParameters.InMaskTexture = Texture2D->GetResource()->TextureRHI;
	PassParameters.InMaskTextureSampler = Texture2D->GetResource()->SamplerStateRHI;
	PassParameters.InGamma = InGamma;
	PassParameters.InClipRef = GAlphaRefVal / 255.0f;
	PassParameters.RenderOpacity = RenderOpacity;
	SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), PassParameters);
}
