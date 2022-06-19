#include "Live2DEditorUtils.h"

#include "Json.h"
#include "JsonObjectConverter.h"

FModel3Data Live2DEditorUtils::CreateModel3DataFromJsonString(const FString& JsonString)
{
	FModel3Data Model3Data;
	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create( JsonString );

	TSharedPtr<FJsonObject> Object;
	if (!FJsonSerializer::Deserialize( Reader, Object ))
	{
		return Model3Data;
	}

	auto FileReferencesJsonObject = Object->GetObjectField("FileReferences");
	
	FJsonObjectConverter::JsonObjectToUStruct<FModel3Data>(FileReferencesJsonObject.ToSharedRef(), &Model3Data, 0, 0);

	auto MotionsJsonObject = FileReferencesJsonObject->GetObjectField("Motions");

	for (auto MotionJsonValue: MotionsJsonObject->Values)
	{
		for (auto MotionFileReference : MotionJsonValue.Value->AsArray())
		{
			FModel3MotionFileData FileData;
			FileData.File = MotionFileReference->AsObject()->GetStringField("File");
			Model3Data.Motions[MotionJsonValue.Key].Motions.Add(FileData) ;
		}
	}

	return Model3Data;
}
