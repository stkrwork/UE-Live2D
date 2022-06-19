#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"

class ULive2DMocModel;

class ILive2DModelEditor : public FAssetEditorToolkit
{
public:
	/** Retrieves the current custom asset. */
	virtual ULive2DMocModel* GetLive2DMocModel() const = 0;

	/** Set the current custom asset. */
	virtual void SetLive2DMocModel(ULive2DMocModel* InLive2DMocModel) = 0;
};
