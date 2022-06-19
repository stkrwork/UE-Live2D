#pragma once

#include "CoreMinimal.h"
#include "Live2DEditorStructs.h"


class Live2DEditorUtils
{
public:
	static FModel3Data CreateModel3DataFromJsonString(const FString& JsonString); 
	
};
