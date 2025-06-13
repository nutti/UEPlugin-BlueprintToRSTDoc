/*!
 * BlueprintToRSTDoc
 *
 * Copyright (c) 2020-2023 nutti
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include "UObject/ObjectMacros.h"

#include "BlueprintToRSTDocSettings.generated.h"

UCLASS(config = BlueprintToRSTDocSettings)
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
