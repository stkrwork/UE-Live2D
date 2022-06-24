// Fill out your copyright notice in the Description page of Project Settings.


#include "Live2DModelPhysics.h"

#include "Live2DMocModel.h"

namespace
{
	constexpr float MaximumWeight = 100.0f;
	constexpr float MovementThreshold = 0.001f;
	constexpr float AirResistance = 5.0f;
}

UWorld* ULive2DModelPhysics::GetWorld() const
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

bool ULive2DModelPhysics::Init(const FPhysics3FileData& Physics3FileData)
{
	EffectiveForces = Physics3FileData.Meta.EffectiveForces;
	PhysicsRigs.Empty();
	PhysicsRigs.Reserve(Physics3FileData.Meta.PhysicsSettingCount);
	for (const auto& Settings : Physics3FileData.PhysicsSettings)
	{
		FLive2DModelPhysicsRig PhysicsRig;
		PhysicsRig.Input = Settings.Input;
		for (const auto& SettingsOutput: Settings.Output)
		{
			FLive2dModelPhysicsOutput Output;
			Output.Destination = SettingsOutput.Destination;
			Output.VertexIndex = SettingsOutput.VertexIndex;
			Output.Scale = SettingsOutput.Scale;
			Output.Weight = SettingsOutput.Weight;
			Output.Type = SettingsOutput.Type;
			Output.bReflect = SettingsOutput.bReflect;
			
			PhysicsRig.Output.Add(Output);
		}
		
		for (const auto& Vertex: Settings.Vertices)
		{
			FLive2dModelPhysicsParticle Particle;
			Particle.Position = Vertex.Position;
			Particle.Mobility = Vertex.Mobility;
			Particle.Delay = Vertex.Delay;
			Particle.Acceleration = Vertex.Acceleration;
			Particle.Radius = Vertex.Radius;
			
			PhysicsRig.Particles.Add(Particle);
		}
		PhysicsRig.Normalization = Settings.Normalization;
		PhysicsRigs.Add(PhysicsRig);
	}

	InitializeParticles();

	return true;
}

void ULive2DModelPhysics::Evaluate(const float DeltaTime)
{
	for (auto& PhysicsRig : PhysicsRigs)
	{
		float TotalAngle = 0.f;
		FVector2D TotalTranslation = FVector2D::ZeroVector;

		for (const auto& Input : PhysicsRig.Input)
		{
			float Weight = Input.Weight / MaximumWeight;
			
			switch (Input.Type)
			{
			case EPhysics3SourceType::X:
				{
					GetInputTranslationXFromNormalizedParameterValue(TotalTranslation, TotalAngle,
						Model->GetParameterValue(Input.Source.Id),
						Model->GetMinimumParameterValue(Input.Source.Id), Model->GetMaximumParameterValue(Input.Source.Id), Model->GetDefaultParameterValue(Input.Source.Id),
						PhysicsRig.Normalization.Position, PhysicsRig.Normalization.Angle,
						Input.bReflect, Weight);
				}
				break;
			case EPhysics3SourceType::Y:
				{
					GetInputTranslationYFromNormalizedParameterValue(TotalTranslation, TotalAngle,
						Model->GetParameterValue(Input.Source.Id),
						Model->GetMinimumParameterValue(Input.Source.Id), Model->GetMaximumParameterValue(Input.Source.Id), Model->GetDefaultParameterValue(Input.Source.Id),
						PhysicsRig.Normalization.Position, PhysicsRig.Normalization.Angle,
						Input.bReflect, Weight);
				}
				break;
			case EPhysics3SourceType::Angle:
				{
					GetInputAngleFromNormalizedParameterValue(TotalTranslation, TotalAngle,
						Model->GetParameterValue(Input.Source.Id),
						Model->GetMinimumParameterValue(Input.Source.Id), Model->GetMaximumParameterValue(Input.Source.Id), Model->GetDefaultParameterValue(Input.Source.Id),
						PhysicsRig.Normalization.Position, PhysicsRig.Normalization.Angle,
						Input.bReflect, Weight);
				}
				break;
			default:
				break;
			}
		}

		float RadAngle = FMath::DegreesToRadians(-TotalAngle);
		
		TotalTranslation.X = (TotalTranslation.X * FMath::Cos(RadAngle) - TotalTranslation.Y * FMath::Sin(RadAngle));
		TotalTranslation.Y = (TotalTranslation.X * FMath::Sin(RadAngle) + TotalTranslation.Y * FMath::Cos(RadAngle));

		UpdateParticles(
			PhysicsRig.Particles,
			PhysicsRig.Particles.Num(),
			TotalTranslation,
			TotalAngle,
			EffectiveForces.Wind,
			MovementThreshold * PhysicsRig.Normalization.Position.Maximum,
			DeltaTime,
			AirResistance
		);

		for (auto& Output: PhysicsRig.Output)
		{
			if (Output.VertexIndex < 1)
			{
				break;
			}

			FVector2D Translation;
			Translation.X = PhysicsRig.Particles[Output.VertexIndex].Position.X - PhysicsRig.Particles[Output.VertexIndex- 1].Position.X;
			Translation.Y = PhysicsRig.Particles[Output.VertexIndex].Position.Y - PhysicsRig.Particles[Output.VertexIndex- 1].Position.Y;

			float OutputValue = 0.f;
			
			switch (Output.Type)
			{
			case EPhysics3SourceType::X:
				{
					OutputValue = GetOutputTranslationX(Translation, PhysicsRig.Particles, Output.VertexIndex, Output.bReflect, EffectiveForces.Gravity);
				}
				break;
			case EPhysics3SourceType::Y:
				{
					OutputValue = GetOutputTranslationY(Translation, PhysicsRig.Particles, Output.VertexIndex, Output.bReflect, EffectiveForces.Gravity);
				}
				break;
			case EPhysics3SourceType::Angle:
				{
					OutputValue = GetOutputAngle(Translation, PhysicsRig.Particles, Output.VertexIndex, Output.bReflect, EffectiveForces.Gravity);
				}
				break;
			default:
				break;
			}

			float ParameterValue;

			UpdateOutputParameterValue(ParameterValue, Model->GetMinimumParameterValue(Output.Destination.Id), Model->GetMaximumParameterValue(Output.Destination.Id), OutputValue, Output);

			Model->SetParameterValue(Output.Destination.Id, ParameterValue);
		}
	}
}

void ULive2DModelPhysics::InitializeParticles()
{
	for (auto& PhysicsRig: PhysicsRigs)
	{
		// Initialize the top of particle.
		PhysicsRig.Particles[0].InitialPosition = FVector2D(0.0f, 0.0f);
		PhysicsRig.Particles[0].LastPosition = PhysicsRig.Particles[0].InitialPosition;
		PhysicsRig.Particles[0].LastGravity = FVector2D(0.0f, -1.0f);
		PhysicsRig.Particles[0].LastGravity.Y *= -1.0f;
		PhysicsRig.Particles[0].Velocity = FVector2D(0.0f, 0.0f);
		PhysicsRig.Particles[0].Force = FVector2D(0.0f, 0.0f);

		for (int32 i = 1; i < PhysicsRig.Particles.Num(); i++)
		{
			FVector2D Radius = FVector2D::ZeroVector;
			Radius.Y = PhysicsRig.Particles[i].Radius;
			PhysicsRig.Particles[i].InitialPosition = PhysicsRig.Particles[i - 1].InitialPosition + Radius;
			PhysicsRig.Particles[i].Position = PhysicsRig.Particles[i].InitialPosition;
			PhysicsRig.Particles[i].LastPosition = PhysicsRig.Particles[i].InitialPosition;
			PhysicsRig.Particles[i].LastGravity = FVector2D(0.0f, -1.0f);
			PhysicsRig.Particles[i].LastGravity.Y *= -1.0f;
			PhysicsRig.Particles[i].Velocity = FVector2D(0.0f, 0.0f);
			PhysicsRig.Particles[i].Force = FVector2D(0.0f, 0.0f);
		}
	}
}

void ULive2DModelPhysics::GetInputTranslationXFromNormalizedParameterValue(FVector2D& TargetTranslation, float& TargetAngle, float Value, float ParameterMinimumValue, float ParameterMaximumValue, float ParameterDefaultValue, const FPhysics3PhysicsRangeData& NormalizationPosition,
                                                                           const FPhysics3PhysicsRangeData& NormalizationAngle, bool bIsInverted, float Weight)
{
	TargetTranslation.X += NormalizeParameterValue(
		Value,
		ParameterMinimumValue,
		ParameterMaximumValue,
		ParameterDefaultValue,
		NormalizationPosition.Minimum,
		NormalizationPosition.Maximum,
		NormalizationPosition.Default,
		bIsInverted
	) * Weight;
}

void ULive2DModelPhysics::GetInputTranslationYFromNormalizedParameterValue(FVector2D& TargetTranslation, float& TargetAngle, float Value, float ParameterMinimumValue, float ParameterMaximumValue, float ParameterDefaultValue, const FPhysics3PhysicsRangeData& NormalizationPosition,
	const FPhysics3PhysicsRangeData& NormalizationAngle, bool bIsInverted, float Weight)
{
	TargetTranslation.Y += NormalizeParameterValue(
		Value,
		ParameterMinimumValue,
		ParameterMaximumValue,
		ParameterDefaultValue,
		NormalizationPosition.Minimum,
		NormalizationPosition.Maximum,
		NormalizationPosition.Default,
		bIsInverted
	) * Weight;
}

void ULive2DModelPhysics::GetInputAngleFromNormalizedParameterValue(FVector2D& TargetTranslation, float& TargetAngle, float Value, float ParameterMinimumValue, float ParameterMaximumValue, float ParameterDefaultValue, const FPhysics3PhysicsRangeData& NormalizationPosition,
	const FPhysics3PhysicsRangeData& NormalizationAngle, bool bIsInverted, float Weight)
{
	TargetTranslation.Y += NormalizeParameterValue(
		Value,
		ParameterMinimumValue,
		ParameterMaximumValue,
		ParameterDefaultValue,
		NormalizationAngle.Minimum,
		NormalizationAngle.Maximum,
		NormalizationAngle.Default,
		bIsInverted
	) * Weight;
}

float ULive2DModelPhysics::NormalizeParameterValue(float Value, float ParameterMinimum, float ParameterMaximum, float ParameterDefault, float NormalizedMinimum, float NormalizedMaximum, float NormalizedDefault, bool bIsInverted)
{
	float Result = 0.0f;

	const float MaxValue = FMath::Max(ParameterMaximum, ParameterMinimum);

	if (MaxValue < Value)
	{
		Value = MaxValue;
	}

	const float MinValue = FMath::Min(ParameterMaximum, ParameterMinimum);

	if (MinValue > Value)
	{
		Value = MinValue;
	}

	const float MinNormValue = FMath::Min(NormalizedMinimum, NormalizedMaximum);
	const float MaxNormValue = FMath::Max(NormalizedMinimum, NormalizedMaximum);
	const float MiddleNormValue = NormalizedDefault;

	const float MiddleValue = MinValue + (FMath::Abs(MaxValue - MinValue) / 2.f);
	const float ParamValue = Value - MiddleValue;

	const int32 SignValue = FMath::Sign(ParamValue);
	switch (SignValue)
	{
	case 1: {
			const float nLength = MaxNormValue - MiddleNormValue;
			const float pLength = MaxValue - MiddleValue;
			if (pLength != 0.0f)
			{
				Result = ParamValue * (nLength / pLength);
				Result += MiddleNormValue;
			}

			break;
	}
	case -1: {
			const float nLength = MinNormValue - MiddleNormValue;
			const float pLength = MinValue - MiddleValue;
			if (pLength != 0.0f)
			{
				Result = ParamValue * (nLength / pLength);
				Result += MiddleNormValue;
			}

			break;
	}
	case 0: {
			Result = MiddleNormValue;

			break;
	}
	default:{
			break;
	}
	}

	return (bIsInverted) ? Result : (Result * -1.0f);
}

float ULive2DModelPhysics::GetOutputTranslationX(const FVector2D& Translation, const TArray<FLive2dModelPhysicsParticle>& Particles, int32 ParticleIndex, bool bIsInverted, FVector2D ParentGravity)
{
	float OutputValue = Translation.X;

	if (bIsInverted)
	{
		OutputValue *= -1.0f;
	}

	return OutputValue;
}

float ULive2DModelPhysics::GetOutputTranslationY(const FVector2D& Translation, const TArray<FLive2dModelPhysicsParticle>& Particles, int32 ParticleIndex, bool bIsInverted, FVector2D ParentGravity)
{
	float OutputValue = Translation.Y;

	if (bIsInverted)
	{
		OutputValue *= -1.0f;
	}

	return OutputValue;
}

float ULive2DModelPhysics::GetOutputAngle(const FVector2D& Translation, const TArray<FLive2dModelPhysicsParticle>& Particles, int32 ParticleIndex, bool bIsInverted, FVector2D ParentGravity)
{
	if (ParticleIndex >= 2)
	{
		ParentGravity = Particles[ParticleIndex - 1].Position - Particles[ParticleIndex - 2].Position;
	}
	else
	{
		ParentGravity *= -1.0f;
	}

	float OutputValue = DirectionToRadian(ParentGravity, Translation);

	if (bIsInverted)
	{
		OutputValue *= -1.0f;
	}

	return OutputValue;
}

void ULive2DModelPhysics::UpdateParticles(TArray<FLive2dModelPhysicsParticle>& Strand, int32 StrandCount, FVector2D TotalTranslation, float TotalAngle, FVector2D WindDirection, float ThresholdValue, float DeltaTimeSeconds, float InAirResistance)
{
	Strand[0].Position = TotalTranslation;

    float TotalRadian = FMath::DegreesToRadians(TotalAngle);
    FVector2D CurrentGravity = FVector2D(FMath::Sin(TotalAngle), FMath::Cos(TotalAngle));
    CurrentGravity.Normalize();

    for (int32 i = 1; i < StrandCount; ++i)
    {
	    FVector2D Velocity;
	    FVector2D Direction;
	    Strand[i].Force = (CurrentGravity * Strand[i].Acceleration) + WindDirection;

        Strand[i].LastPosition = Strand[i].Position;

        float Delay = Strand[i].Delay * DeltaTimeSeconds * 30.0f;

        Direction.X = Strand[i].Position.X - Strand[i - 1].Position.X;
        Direction.Y = Strand[i].Position.Y - Strand[i - 1].Position.Y;

        float Radian = DirectionToRadian(Strand[i].LastGravity, CurrentGravity) / InAirResistance;

        Direction.X = ((FMath::Cos(Radian) * Direction.X) - (Direction.Y * FMath::Sin(Radian)));
        Direction.Y = ((FMath::Sin(Radian) * Direction.X) + (Direction.Y * FMath::Cos(Radian)));

        Strand[i].Position = Strand[i - 1].Position + Direction;

        Velocity.X = Strand[i].Velocity.X * Delay;
        Velocity.Y = Strand[i].Velocity.Y * Delay;
        FVector2D Force = Strand[i].Force * Delay * Delay;

        Strand[i].Position = Strand[i].Position + Velocity + Force;

        FVector2D NewDirection = Strand[i].Position - Strand[i - 1].Position;

        NewDirection.Normalize();

        Strand[i].Position = Strand[i - 1].Position + (NewDirection * Strand[i].Radius);

        if (FMath::Abs(Strand[i].Position.X) < ThresholdValue)
        {
            Strand[i].Position.X = 0.0f;
        }

        if (Delay != 0.0f)
        {
            Strand[i].Velocity.X = Strand[i].Position.X - Strand[i].LastPosition.X;
            Strand[i].Velocity.Y = Strand[i].Position.Y - Strand[i].LastPosition.Y;
            Strand[i].Velocity /= Delay;
            Strand[i].Velocity *= Strand[i].Mobility;
        }

        Strand[i].Force = FVector2D( 0.0f, 0.0f );
        Strand[i].LastGravity = CurrentGravity;
    }
}

void ULive2DModelPhysics::UpdateOutputParameterValue(float& ParameterValue, float ParameterValueMinimum, float ParameterValueMaximum, float Translation, FLive2dModelPhysicsOutput& Output)
{
	float OutputScale;

	switch (Output.Type)
	{
	case EPhysics3SourceType::X:
		{
			OutputScale = 0.f; // Output.TranslationScale.X; this is not set anywhere in the original framework implementation
		}
		break;
	case EPhysics3SourceType::Y:
		{
			OutputScale = 0.f; // Output.TranslationScale.Y; this is not set anywhere in the original framework implementation
		}
		break;
	case EPhysics3SourceType::Angle:
		{
			OutputScale = Output.Scale;
		}
		break;
	default:
		return;
	}

	float Value = Translation * OutputScale;

	if (Value < ParameterValueMinimum)
	{
		if (Value < Output.ValueBelowMinimum)
		{
			Output.ValueBelowMinimum = Value;
		}

		Value = ParameterValueMinimum;
	}
	else if (Value > ParameterValueMaximum)
	{
		if (Value > Output.ValueExceededMaximum)
		{
			Output.ValueExceededMaximum = Value;
		}

		Value = ParameterValueMaximum;
	}

	float Weight = (Output.Weight / MaximumWeight);

	if (Weight >= 1.0f)
	{
		ParameterValue = Value;
	}
	else
	{
		Value = (ParameterValue * (1.0f - Weight)) + (Value * Weight);
		ParameterValue = Value;
	}
}

float ULive2DModelPhysics::DirectionToRadian(const FVector2D& A, const FVector2D& B)
{
	return FMath::Acos(FVector2D::DotProduct(A,  B) / (A.Size() * B.Size()));
}
