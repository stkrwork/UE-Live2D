#include "Live2DModelMotionCurve.h"

bool FLive2DModelMotionCurve::Init(const FMotion3CurveData& CurveData, const FMotion3MetaData& MetaData)
{
	if (CurveData.Target == TEXT("Model"))
	{
		Target = ECurveTarget::TARGET_MODEL;
	}
	else if (CurveData.Target == TEXT("Parameter"))
	{
		Target = ECurveTarget::TARGET_PARAMETER;
	}
	else if (CurveData.Target == TEXT("PartOpacity"))
	{
		Target = ECurveTarget::TARGET_PART_OPACITY;
	}

	FadeInTime = CurveData.FadeInTime;
	FadeOutTime = CurveData.FadeOutTime;
	
	Id = CurveData.Id;
	
	for (int32 i = 0; i < CurveData.Segments.Num();)
	{
		FCurveSegment Segment;
		FSegmentAnimationPoint Point;

		if (i == 0)
		{
			Segment.PointIndex = 0;
			FSegmentAnimationPoint FirstPoint;
			FirstPoint.Time = CurveData.Segments[0];
			FirstPoint.Value = CurveData.Segments[1];
			Points.Add(FirstPoint);
			i += 2;
		}
		else
		{
			Segment.PointIndex = Points.Num() - 1;
		}
		
		Segment.SegmentType = static_cast<ECurveSegmentType>(CurveData.Segments[i++]);

		switch (Segment.SegmentType)
		{
		case ECurveSegmentType::LINEAR_SEGMENT:
			{
				Segment.EvaluateDelegate.BindStatic(ULive2DSegmentEvaluationUtilities::EvaluateLinear);
				Point.Time = CurveData.Segments[i++];
				Point.Value = CurveData.Segments[i++];
				Points.Add(Point);
				
				Segments.Add(Segment);
			}
			break;
		case ECurveSegmentType::BEZIER_SEGMENT:
			{
				if (MetaData.bAreBeziersRestricted)
				{
					Segment.EvaluateDelegate.BindStatic(ULive2DSegmentEvaluationUtilities::EvaluateBezier);
				}
				else
				{
					Segment.EvaluateDelegate.BindStatic(ULive2DSegmentEvaluationUtilities::EvaluateBezierCardanoInterpretation);					
				}
				Point.Time = CurveData.Segments[i++];
				Point.Value = CurveData.Segments[i++];
				Points.Add(Point);
				Point.Time = CurveData.Segments[i++];
				Point.Value = CurveData.Segments[i++];
				Points.Add(Point);
				Point.Time = CurveData.Segments[i++];
				Point.Value = CurveData.Segments[i++];
				Points.Add(Point);
				Segments.Add(Segment);
			}
			break;
		case ECurveSegmentType::STEPPED_SEGMENT:
			{
				Segment.EvaluateDelegate.BindStatic(ULive2DSegmentEvaluationUtilities::EvaluateStepped);
				Point.Time = CurveData.Segments[i++];
				Point.Value = CurveData.Segments[i++];
				Points.Add(Point);
				Segments.Add(Segment);
			}
			break;
		case ECurveSegmentType::INVERSE_STEPPED_SEGMENT:
			{
				Segment.EvaluateDelegate.BindStatic(ULive2DSegmentEvaluationUtilities::EvaluateInverseStepped);
				Point.Time = CurveData.Segments[i++];
				Point.Value = CurveData.Segments[i++];
				Points.Add(Point);
				Segments.Add(Segment);
			}
			break;
		}
	}

	return true;
}

void FLive2DModelMotionCurve::RebindDelegates(const bool bAreBeziersRestricted)
{	
	for (int32 i = 0; i < Segments.Num(); i++)
	{
		FCurveSegment& CurveSegment = Segments[i];
		switch (CurveSegment.SegmentType)
		{
		case ECurveSegmentType::LINEAR_SEGMENT:
			{
				CurveSegment.EvaluateDelegate.BindStatic(ULive2DSegmentEvaluationUtilities::EvaluateLinear);
			}
			break;
		case ECurveSegmentType::BEZIER_SEGMENT:
			{
				if (bAreBeziersRestricted)
				{
					CurveSegment.EvaluateDelegate.BindStatic(ULive2DSegmentEvaluationUtilities::EvaluateBezier);
				}
				else
				{
					CurveSegment.EvaluateDelegate.BindStatic(ULive2DSegmentEvaluationUtilities::EvaluateBezierCardanoInterpretation);					
				}
			}
			break;
		case ECurveSegmentType::STEPPED_SEGMENT:
			{
				CurveSegment.EvaluateDelegate.BindStatic(ULive2DSegmentEvaluationUtilities::EvaluateStepped);
			}
			break;
		case ECurveSegmentType::INVERSE_STEPPED_SEGMENT:
			{
				CurveSegment.EvaluateDelegate.BindStatic(ULive2DSegmentEvaluationUtilities::EvaluateInverseStepped);
			}
			break;
		}
	}
}

void FLive2DModelMotionCurve::UpdateParameter(ULive2DMocModel* Model, const float Time)
{
	float Value = 0.f;
	for (int32 i = 0; i < Segments.Num(); i++)
	{
		FCurveSegment CurveSegment = Segments[i];
		auto Point = Points[CurveSegment.PointIndex];
		if (Point.Time > Time)
		{
			Value = CurveSegment.EvaluateDelegate.Execute(&Points[CurveSegment.PointIndex], Time);
			break;
		}

		if (i == Segments.Num() -1)
		{
			Value = Point.Value;
		}
	}

	Model->SetParameterValue(Id, Value);
}
