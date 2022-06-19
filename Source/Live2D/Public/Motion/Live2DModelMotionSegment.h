// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Live2DModelMotionSegment.generated.h"

UENUM(BlueprintType)
enum class ECurveSegmentType: uint8
{
	LINEAR_SEGMENT,
	BEZIER_SEGMENT,
	STEPPED_SEGMENT,
	INVERSE_STEPPED_SEGMENT
};

USTRUCT(BlueprintType)
struct FSegmentAnimationPoint
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Time;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Value;
};

DECLARE_DELEGATE_RetVal_TwoParams(float, FEvaluateDelegate, const FSegmentAnimationPoint* /* Points */, const float /* Time */)

USTRUCT(BlueprintType)
struct FCurveSegment
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECurveSegmentType SegmentType;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PointIndex;
	
	FEvaluateDelegate EvaluateDelegate;	
};

/**
 * 
 */
UCLASS()
class LIVE2D_API ULive2DSegmentEvaluationUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static float EvaluateLinear(const FSegmentAnimationPoint* Points, const float Time);
	static float EvaluateBezier(const FSegmentAnimationPoint* Points, const float Time);
	static float EvaluateBezierCardanoInterpretation(const FSegmentAnimationPoint* Points, const float Time);
	static float EvaluateStepped(const FSegmentAnimationPoint* Points, const float Time);
	static float EvaluateInverseStepped(const FSegmentAnimationPoint* Points, const float Time);

protected:
	static FSegmentAnimationPoint LerpPoints(const FSegmentAnimationPoint& A, const FSegmentAnimationPoint& B, const float Alpha);
	static float CardanoAlgorithmForBezier(float a, float b, float c, float d);
	static float QuadraticEquation(float a, float b, float c);
};

