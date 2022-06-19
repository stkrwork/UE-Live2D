// Fill out your copyright notice in the Description page of Project Settings.


#include "Live2DModelMotionSegment.h"

float ULive2DSegmentEvaluationUtilities::EvaluateLinear(const FSegmentAnimationPoint* Points, const float Time)
{
	const float t = (Time - Points[0].Time) / (Points[1].Time - Points[0].Time);
	return Points[0].Value + ((Points[1].Value - Points[0].Value) * t);
}

float ULive2DSegmentEvaluationUtilities::EvaluateBezier(const FSegmentAnimationPoint* Points, const float Time)
{
	FSegmentAnimationPoint p01, p12, p23, p012, p123;
	float t = (Time - Points[0].Time) / (Points[3].Time - Points[0].Time);


	p01 = LerpPoints(Points[0], Points[1], t);
	p12 = LerpPoints(Points[1], Points[2], t);
	p23 = LerpPoints(Points[2], Points[3], t);

	p012 = LerpPoints(p01, p12, t);
	p123 = LerpPoints(p12, p23, t);


	return LerpPoints(p012, p123, t).Value;
}

float ULive2DSegmentEvaluationUtilities::EvaluateBezierCardanoInterpretation(const FSegmentAnimationPoint* Points, const float Time)
{
	const float x = Time;
	float x1 = Points[0].Time;
	float x2 = Points[3].Time;
	float cx1 = Points[1].Time;
	float cx2 = Points[2].Time;

	float a = x2 - 3.0f * cx2 + 3.0f * cx1 - x1;
	float b = 3.0f * cx2 - 6.0f * cx1 + 3.0f * x1;
	float c = 3.0f * cx1 - 3.0f * x1;
	float d = x1 - x;

	float t = CardanoAlgorithmForBezier(a, b, c, d);

	const FSegmentAnimationPoint p01 = LerpPoints(Points[0], Points[1], t);
	const FSegmentAnimationPoint p12 = LerpPoints(Points[1], Points[2], t);
	const FSegmentAnimationPoint p23 = LerpPoints(Points[2], Points[3], t);

	const FSegmentAnimationPoint p012 = LerpPoints(p01, p12, t);
	const FSegmentAnimationPoint p123 = LerpPoints(p12, p23, t);

	return LerpPoints(p012, p123, t).Value;
}

float ULive2DSegmentEvaluationUtilities::EvaluateStepped(const FSegmentAnimationPoint* Points, const float Time)
{
	return Points[0].Value;
}

float ULive2DSegmentEvaluationUtilities::EvaluateInverseStepped(const FSegmentAnimationPoint* Points, const float Time)
{
	return Points[1].Value;
}

FSegmentAnimationPoint ULive2DSegmentEvaluationUtilities::LerpPoints(const FSegmentAnimationPoint& A, const FSegmentAnimationPoint& B, const float Alpha)
{
	FSegmentAnimationPoint NewPoint;

	NewPoint.Time = A.Time + ((B.Time - A.Time) * Alpha);
	NewPoint.Value = A.Value + ((B.Value - A.Value) * Alpha);

	return NewPoint;
}

float ULive2DSegmentEvaluationUtilities::CardanoAlgorithmForBezier(float a, float b, float c, float d)
{
	  if ( FMath::Abs( a ) < FLT_EPSILON )
    {
        return FMath::Clamp( QuadraticEquation(b, c, d), 0.0f, 1.0f);
    }

    float ba = b / a;
    float ca = c / a;
    float da = d / a;


    float p = (3.0f * ca - ba*ba) / 3.0f;
    float p3 = p / 3.0f;
    float q = (2.0f * ba*ba*ba - 9.0f * ba*ca + 27.0f * da) / 27.0f;
    float q2 = q / 2.0f;
    float discriminant = q2*q2 + p3*p3*p3;

    const float center = 0.5f;
    const float threshold = center + 0.01f;

    if (discriminant < 0.0f) {
        float mp3 = -p / 3.0f;
        float mp33 = mp3*mp3*mp3;
        float r = FMath::Sqrt(mp33);
        float t = -q / (2.0f * r);
        float cosphi = FMath::Clamp(t, -1.0f, 1.0f);
        float phi = acos(cosphi);
        float crtr = cbrt(r);
        float t1 = 2.0f * crtr;

        float root1 = t1 * FMath::Cos(phi / 3.0f) - ba / 3.0f;
        if ( abs( root1 - center) < threshold)
        {
            return FMath::Clamp( root1, 0.0f, 1.0f);
        }

        float root2 = t1 * FMath::Cos((phi + 2.0f * PI) / 3.0f) - ba / 3.0f;
        if (abs(root2 - center) < threshold)
        {
            return FMath::Clamp(root2, 0.0f, 1.0f);
        }

        float root3 = t1 * FMath::Cos((phi + 4.0f * PI) / 3.0f) - ba / 3.0f;
        return FMath::Clamp(root3, 0.0f, 1.0f);
    }

    if (discriminant == 0.0f) {
        float u1;
        if (q2 < 0.0f)
        {
        	
            u1 = FMath::Pow(-q2, 1/3);
        }
        else
        {
            u1 = - FMath::Pow(q2, 1/3);
        }

        float root1 = 2.0f * u1 - ba / 3.0f;
        if (FMath::Abs(root1 - center) < threshold)
        {
            return FMath::Clamp(root1, 0.0f, 1.0f);
        }

        float root2 = -u1 - ba / 3.0f;
        return FMath::Clamp(root2, 0.0f, 1.0f);
    }

    float sd = FMath::Sqrt(discriminant);
    float u1 = cbrt(sd - q2);
    float v1 = cbrt(sd + q2);
    float root1 = u1 - v1 - ba / 3.0f;
    return FMath::Clamp(root1, 0.0f, 1.0f);
}

float ULive2DSegmentEvaluationUtilities::QuadraticEquation(float a, float b, float c)
{
	if (FMath::Abs(a) < FLT_EPSILON)
	{
		if (FMath::Abs(b) < FLT_EPSILON)
		{
			return -c;
		}
		return -c / b;
	}

	return -(b + FMath::Sqrt(b * b - 4.0f * a * c)) / (2.0f * a);
}
