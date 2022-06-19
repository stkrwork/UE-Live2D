#include "Live2DModelMotion.h"

bool ULive2DModelMotion::Init(const FMotion3FileData& Motion3Data)
{
	Duration = Motion3Data.Meta.Duration;
	bLoop = Motion3Data.Meta.bLoop;
	FPS = Motion3Data.Meta.FPS;
	bAreBeziersRestricted = Motion3Data.Meta.bAreBeziersRestricted;

	for (const auto& Curve: Motion3Data.Curves)
	{
		FLive2DModelMotionCurve MotionCurve;
		MotionCurve.Init(Curve, Motion3Data.Meta);
		Curves.Add(MotionCurve);
	}

	return true;
}

void ULive2DModelMotion::RebindDelegates()
{
	for (auto& Curve: Curves)
	{
		Curve.RebindDelegates(bAreBeziersRestricted);
	}	
}

void ULive2DModelMotion::Tick(float DeltaTime)
{
	CurrentTime = FMath::Min(Duration, CurrentTime + DeltaTime);
	for (auto& Curve: Curves)
	{
		if (Curve.Target == ECurveTarget::TARGET_PARAMETER)
		{
			Curve.UpdateParameter(Model, CurrentTime);
		}
	}

	if (FMath::IsNearlyEqual(CurrentTime, Duration) && bLoop)
	{
		CurrentTime = 0.f;
	}
}

TStatId ULive2DModelMotion::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FLive2DModelMotion, STATGROUP_Live2D);
}

void ULive2DModelMotion::ToggleMotionInEditor()
{
	bIsAnimating = !bIsAnimating;	
}
