#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"

class ULive2DModelMotion;

class ILive2DMotionEditor : public FAssetEditorToolkit
{
public:
	/** Retrieves the current custom asset. */
	virtual ULive2DModelMotion* GetLive2DModelMotion() const = 0;

	/** Set the current custom asset. */
	virtual void SetLive2DModelMotion(ULive2DModelMotion* InLive2DModelMotion) = 0;
};
