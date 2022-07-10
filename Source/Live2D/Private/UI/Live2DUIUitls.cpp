// Fill out your copyright notice in the Description page of Project Settings.


#include "Live2DUIUitls.h"

#include "Components/Image.h"
#include "Engine/AssetManager.h"

void ULive2DUIUitls::SetBrushFromLive2DModelMotion(UImage* Image, ULive2DModelMotion* ModelMotion)
{
	Image->SetBrush(ModelMotion->GetModel()->GetImageBrush());
}

void ULive2DUIUitls::SetBrushFromSoftLive2DModelMotion(UImage* Image, TSoftObjectPtr<ULive2DModelMotion> ModelMotion)
{
	if (ULive2DModelMotion* StrongModelMotion = ModelMotion.Get())
	{
		SetBrushFromLive2DModelMotion(Image, StrongModelMotion);
		return;  // No streaming was needed, complete immediately.
	}

	TWeakObjectPtr<UImage> WeakImage(Image);
	FSoftObjectPath StreamingObjectPath = ModelMotion.ToSoftObjectPath();
	UAssetManager::GetStreamableManager().RequestAsyncLoad(
		StreamingObjectPath,
		[WeakImage, ModelMotion, StreamingObjectPath]() {
			if (UImage* StrongImage = WeakImage.Get())
			{
				// If the object paths don't match, then this delegate was interrupted, but had already been queued for a callback
				// so ignore everything and abort.
				if (StreamingObjectPath != ModelMotion.ToSoftObjectPath())
				{
					return; // Abort!
				}

				SetBrushFromLive2DModelMotion(StrongImage, ModelMotion.Get());
			}
		},
		FStreamableManager::AsyncLoadHighPriority);
}
