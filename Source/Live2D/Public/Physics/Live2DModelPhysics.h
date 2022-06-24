// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Live2DStructs.h"
#include "UObject/Object.h"
#include "Live2DModelPhysics.generated.h"


class ULive2DMocModel;
USTRUCT(BlueprintType)
struct FLive2dModelPhysicsOutput
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FPhysics3PhysicsInOutputPinData Destination;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 VertexIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Scale;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Weight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EPhysics3SourceType Type;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bReflect;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ValueBelowMinimum;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ValueExceededMaximum;

	
};
USTRUCT(BlueprintType)
struct FLive2dModelPhysicsParticle
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D InitialPosition;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)     
	float Mobility;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Delay;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Acceleration;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Radius;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)   
	FVector2D Position;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D LastPosition;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D LastGravity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D Force;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D Velocity;
};

USTRUCT(BlueprintType)
struct FLive2DModelPhysicsRig
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FPhysics3PhysicsNormalizationData Normalization;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FPhysics3PhysicsInputData> Input;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FLive2dModelPhysicsOutput> Output;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FLive2dModelPhysicsParticle> Particles;
};

/**
 * 
 */
UCLASS()
class LIVE2D_API ULive2DModelPhysics : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;
	bool Init(const FPhysics3FileData& Physics3FileData);
	void SetModel(ULive2DMocModel* InModel) { Model = InModel; }

	void Evaluate(const float DeltaTime);

protected:
	void InitializeParticles();
	void GetInputTranslationXFromNormalizedParameterValue(FVector2D& TargetTranslation, float& TargetAngle, float Value,
		float ParameterMinimumValue, float ParameterMaximumValue, float ParameterDefaultValue,
		const FPhysics3PhysicsRangeData& NormalizationPosition, const FPhysics3PhysicsRangeData& NormalizationAngle,
		bool bIsInverted, float Weight);
	void GetInputTranslationYFromNormalizedParameterValue(FVector2D& TargetTranslation, float& TargetAngle, float Value,
		float ParameterMinimumValue, float ParameterMaximumValue, float ParameterDefaultValue,
		const FPhysics3PhysicsRangeData& NormalizationPosition, const FPhysics3PhysicsRangeData& NormalizationAngle,
		bool bIsInverted, float Weight);
	void GetInputAngleFromNormalizedParameterValue(FVector2D& TargetTranslation, float& TargetAngle, float Value,
		float ParameterMinimumValue, float ParameterMaximumValue, float ParameterDefaultValue,
		const FPhysics3PhysicsRangeData& NormalizationPosition, const FPhysics3PhysicsRangeData& NormalizationAngle,
		bool bIsInverted, float Weight);
	
	float NormalizeParameterValue(float Value,
		float ParameterMinimum,	float ParameterMaximum,	float ParameterDefault,
		float NormalizedMinimum, float NormalizedMaximum, float NormalizedDefault,
		bool bIsInverted);

	float GetOutputTranslationX(const FVector2D& Translation, const TArray<FLive2dModelPhysicsParticle>& Particles, int32 ParticleIndex,
	bool bIsInverted, FVector2D ParentGravity);

	float GetOutputTranslationY(const FVector2D& Translation, const TArray<FLive2dModelPhysicsParticle>& Particles, int32 ParticleIndex,
	bool bIsInverted, FVector2D ParentGravity);

	float GetOutputAngle(const FVector2D& Translation, const TArray<FLive2dModelPhysicsParticle>& Particles, int32 ParticleIndex,
	bool bIsInverted, FVector2D ParentGravity);

	void UpdateParticles(TArray<FLive2dModelPhysicsParticle>& Strand, int32 StrandCount, FVector2D TotalTranslation, float TotalAngle,
	FVector2D WindDirection, float ThresholdValue, float DeltaTimeSeconds, float InAirResistance);

	void UpdateOutputParameterValue(float& ParameterValue, float ParameterValueMinimum, float ParameterValueMaximum,
	float Translation, FLive2dModelPhysicsOutput& Output);

	float DirectionToRadian(const FVector2D& A, const FVector2D& B);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FLive2DModelPhysicsRig> PhysicsRigs;

	UPROPERTY()
	ULive2DMocModel* Model;

	UPROPERTY()
	FPhysics3EffectiveForcesData EffectiveForces;
};
