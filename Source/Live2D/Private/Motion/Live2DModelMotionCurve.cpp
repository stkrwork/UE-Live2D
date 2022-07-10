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
			FirstPoint.Time = CurveData.Segments[i++];
			FirstPoint.Value = CurveData.Segments[i++];
			Points.Add(FirstPoint);
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
				Point.Time = CurveData.Segments[i++];
				Point.Value = CurveData.Segments[i++];
				Points.Add(Point);
				
				Segments.Add(Segment);
			}
			break;
		case ECurveSegmentType::BEZIER_SEGMENT:
			{
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
				Point.Time = CurveData.Segments[i++];
				Point.Value = CurveData.Segments[i++];
				Points.Add(Point);
				Segments.Add(Segment);
			}
			break;
		case ECurveSegmentType::INVERSE_STEPPED_SEGMENT:
			{
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

void FLive2DModelMotionCurve::UpdateParameter(ULive2DMocModel* Model, const float Time, const bool bAreBeziersRestricted)
{
	float Value = 0.f;
	for (int32 i = 0; i < Segments.Num() - 1; i++)
	{
		FCurveSegment CurveSegment = Segments[i];
		FCurveSegment NextCurveSegment = Segments[i+1];
		const FSegmentAnimationPoint NextSegmentPoint = Points[NextCurveSegment.PointIndex];
		if (NextSegmentPoint.Time > Time)
		{
			switch (CurveSegment.SegmentType)
			{
			case ECurveSegmentType::LINEAR_SEGMENT:
				{
					ULive2DSegmentEvaluationUtilities::EvaluateLinear(&Points[CurveSegment.PointIndex], Time);
				}
				break;
			case ECurveSegmentType::BEZIER_SEGMENT:
				{
					if (bAreBeziersRestricted)
					{
						ULive2DSegmentEvaluationUtilities::EvaluateBezier(&Points[CurveSegment.PointIndex], Time);
					}
					else
					{
						ULive2DSegmentEvaluationUtilities::EvaluateBezierCardanoInterpretation(&Points[CurveSegment.PointIndex], Time);					
					}
				}
				break;
			case ECurveSegmentType::STEPPED_SEGMENT:
				{
					ULive2DSegmentEvaluationUtilities::EvaluateStepped(&Points[CurveSegment.PointIndex], Time);
				}
				break;
			case ECurveSegmentType::INVERSE_STEPPED_SEGMENT:
				{
					ULive2DSegmentEvaluationUtilities::EvaluateInverseStepped(&Points[CurveSegment.PointIndex], Time);
				}
				break;
			}
			break;
		}

		if (i == Segments.Num() - 2)
		{
			Value = NextSegmentPoint.Value;
		}
	}

	Model->SetParameterValue(Id, Value);
}

void FLive2DModelMotionCurve::UpdatePartOpacity(ULive2DMocModel* Model, const float Time, const bool bAreBeziersRestricted)
{
	float Value = 0.f;
	for (int32 i = 0; i < Segments.Num(); i++)
	{
		FCurveSegment CurveSegment = Segments[i];
		auto Point = Points[CurveSegment.PointIndex];
		if (Point.Time > Time)
		{
			switch (CurveSegment.SegmentType)
			{
			case ECurveSegmentType::LINEAR_SEGMENT:
				{
					ULive2DSegmentEvaluationUtilities::EvaluateLinear(&Points[CurveSegment.PointIndex], Time);
				}
				break;
			case ECurveSegmentType::BEZIER_SEGMENT:
				{
					if (bAreBeziersRestricted)
					{
						ULive2DSegmentEvaluationUtilities::EvaluateBezier(&Points[CurveSegment.PointIndex], Time);
					}
					else
					{
						ULive2DSegmentEvaluationUtilities::EvaluateBezierCardanoInterpretation(&Points[CurveSegment.PointIndex], Time);					
					}
				}
				break;
			case ECurveSegmentType::STEPPED_SEGMENT:
				{
					ULive2DSegmentEvaluationUtilities::EvaluateStepped(&Points[CurveSegment.PointIndex], Time);
				}
				break;
			case ECurveSegmentType::INVERSE_STEPPED_SEGMENT:
				{
					ULive2DSegmentEvaluationUtilities::EvaluateInverseStepped(&Points[CurveSegment.PointIndex], Time);
				}
				break;
			}
			break;
		}

		if (i == Segments.Num() -1)
		{
			Value = Point.Value;
		}
	}

	Model->SetPartOpacityValue(Id, Value);
}
