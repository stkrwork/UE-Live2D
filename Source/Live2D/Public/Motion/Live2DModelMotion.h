#pragma once

#include "CoreMinimal.h"
#include "Live2DMocModel.h"
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
	virtual UWorld* GetWorld() const override;
	
	bool Init(const FMotion3FileData& Motion3Data);
	void RebindDelegates();
	
	void SetModel(ULive2DMocModel* InModel) { Model = InModel;}
	ULive2DMocModel* GetModel() const { return Model; }

	UFUNCTION(CallInEditor)
	void ToggleMotionInEditor();

protected:

	void ToggleTimer();

	UFUNCTION()
	void Tick(const float InDeltaTime);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ULive2DMocModel* Model;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Duration;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float FPS;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bLoop;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bAreBeziersRestricted = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FLive2DModelMotionCurve> Curves;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DeltaTime = 0.f;
	
	float CurrentTime = 0.f;

	bool bIsAnimating = false;
	FTimerHandle Timer;
};
