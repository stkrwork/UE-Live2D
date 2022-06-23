#include "Live2DModelMotion.h"

UWorld* ULive2DModelMotion::GetWorld() const
{
	// This implementation is needed so the blueprint can access worldcontext methods from blueprintFunctionLibraries. The check for for the defaultObject is there because in the editor the object doesn't have an outer.
	// A more detailed explanation is here: https://answers.unrealengine.com/questions/468741/how-to-make-a-blueprint-derived-from-a-uobject-cla.html
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		return GetOuter()->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

bool ULive2DModelMotion::Init(const FMotion3FileData& Motion3Data)
{
	Duration = Motion3Data.Meta.Duration;
	bLoop = Motion3Data.Meta.bLoop;
	FPS = Motion3Data.Meta.FPS;
	DeltaTime = 1.f/FPS;
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

void ULive2DModelMotion::Tick(float Delta)
{
	TimeSinceLastDelta += Delta;

	if (TimeSinceLastDelta < DeltaTime)
	{
		return;
	}

	TimeSinceLastDelta = 0.f;
	
	CurrentTime = FMath::Min(Duration, CurrentTime + Delta);
	for (auto& Curve: Curves)
	{
		if (Curve.Target == ECurveTarget::TARGET_PARAMETER)
		{
			Curve.UpdateParameter(Model, CurrentTime);
		}
		else if (Curve.Target == ECurveTarget::TARGET_PART_OPACITY)
		{
			Curve.UpdatePartOpacity(Model, CurrentTime);
		}
	}

	Model->UpdateDrawables();

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
	DeltaTime = 1.f/FPS;
	bIsAnimating = !bIsAnimating;	
}
