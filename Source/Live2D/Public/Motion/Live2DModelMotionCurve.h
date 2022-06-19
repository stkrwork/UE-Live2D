#pragma once

#include "CoreMinimal.h"
#include "Live2DModelMotionSegment.h"
#include "Live2DStructs.h"

#include "Live2DModelMotionCurve.generated.h"

UENUM(BlueprintType)
enum class ECurveTarget: uint8
{
	INVALID_TARGET,
	TARGET_MODEL,
	TARGET_PARAMETER,
	TARGET_PART_OPACITY
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FLive2DModelMotionCurve
{
	GENERATED_BODY()
public:
	bool Init(const FMotion3CurveData& CurveData);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECurveTarget Target = ECurveTarget::INVALID_TARGET;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Id;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FCurveSegment> Segments;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FSegmentAnimationPoint> Points;
};
