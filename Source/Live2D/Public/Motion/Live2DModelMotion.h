#pragma once

#include "CoreMinimal.h"
#include "Live2DModelMotionCurve.h"

#include "Live2DModelMotion.generated.h"


/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LIVE2D_API ULive2DModelMotion : public UObject
{
	GENERATED_BODY()
public:
	bool Init(const FMotion3FileData& Motion3Data);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Duration;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float FPS;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bLoop;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FLive2DModelMotionCurve> Curves;
};
