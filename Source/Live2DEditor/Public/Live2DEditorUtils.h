#pragma once

#include "CoreMinimal.h"
#include "Live2DStructs.h"


class Live2DEditorUtils
{
public:
	static FModel3Data CreateModel3DataFromJsonString(const FString& JsonString); 
	static FMotion3FileData CreateMotion3FileDataFromJsonString(const FString& JsonString); 
	
};
