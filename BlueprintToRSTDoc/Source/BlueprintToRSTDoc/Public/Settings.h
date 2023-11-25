#pragma once

#include "UObject/ObjectMacros.h"

#include "Settings.generated.h"

UCLASS(config = BlueprintToDocUserSettings)
class UBlueprintToRSTDocSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = "Path", meta = (RelativePath))
	TArray<FString> ExcludePaths;

	UPROPERTY(config, EditAnywhere, Category = "Path")
	bool bAlwaysAskOutputDirectory = true;

	UPROPERTY(config, EditAnywhere, Category = "Path", meta = (RelativePath, EditCondition = "!bAlwaysAskOutputDirectory"))
	FString OutputDirectory = "";

	UPROPERTY(config, EditAnywhere, Category = "Output")
	bool bOutputBlueprint = true;

	UPROPERTY(config, EditAnywhere, Category = "Output")
	bool bOutputStructure = true;

	UPROPERTY(config, EditAnywhere, Category = "Output")
	bool bOutputEnumeration = true;

	UPROPERTY(config, EditAnywhere, Category = "Utility")
	bool bOutputDocsList = true;

	UPROPERTY(config, EditAnywhere, Category = "Utility", meta = (RelativePath, EditCondition = "bOutputDocsList"))
	FString OutputDocsListFileName = "DocsList.txt";

	UPROPERTY(config, EditAnywhere, Category = "Utility", meta = (EditCondition = "bOutputDocsList"))
	bool bOutputDocsListFullPath = false;
};
