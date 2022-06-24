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

void ULive2DModelMotion::ToggleMotionInEditor()
{
	DeltaTime = 1.f/FPS;
	ToggleTimer();
}

void ULive2DModelMotion::ToggleTimer()
{	
	if (Model->IsTicking())
	{
		Model->StopTicking();
		Model->OnModelTick.RemoveDynamic(this, &ULive2DModelMotion::Tick);
	}
	else
	{
		Model->OnModelTick.AddUniqueDynamic(this, &ULive2DModelMotion::Tick);
		Model->StartTicking(DeltaTime);
	}
}

void ULive2DModelMotion::Tick(const float InDeltaTime)
{
	CurrentTime = FMath::Min(Duration, CurrentTime + InDeltaTime);
	for (auto& Curve: Curves)
	{
		if (Curve.Target == ECurveTarget::TARGET_PARAMETER || Curve.Target == ECurveTarget::TARGET_MODEL)
		{
			Curve.UpdateParameter(Model, CurrentTime);
		}
		else if (Curve.Target == ECurveTarget::TARGET_PART_OPACITY)
		{
			Curve.UpdatePartOpacity(Model, CurrentTime);
		}
	}

	if (FMath::IsNearlyEqual(CurrentTime, Duration) && bLoop)
	{
		if (bLoop)
		{
			CurrentTime = 0.f;
		}
		else
		{
			ToggleTimer();
		}
	}
}
