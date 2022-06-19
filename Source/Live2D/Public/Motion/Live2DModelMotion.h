#pragma once

#include "CoreMinimal.h"
#include "Live2DMocModel.h"
#include "Live2DModelMotionCurve.h"

#include "Live2DModelMotion.generated.h"


/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LIVE2D_API ULive2DModelMotion : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
public:
	bool Init(const FMotion3FileData& Motion3Data);
	void RebindDelegates();
	
	void SetModel(ULive2DMocModel* InModel) { Model = InModel;}
	ULive2DMocModel* GetModel() const { return Model; }

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableInEditor() const override { return true; };
	virtual bool IsTickable() const override { return bIsAnimating; };

	UFUNCTION(CallInEditor)
	void ToggleMotionInEditor();

protected:
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

	float CurrentTime = 0.f;

	bool bIsAnimating = false;
};
