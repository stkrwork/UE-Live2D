#include "Live2DModelMotionCurve.h"

bool FLive2DModelMotionCurve::Init(const FMotion3CurveData& CurveData)
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
				// TODO add support for cardano algo
				Segment.EvaluateDelegate.BindStatic(ULive2DSegmentEvaluationUtilities::EvaluateBezier);
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
