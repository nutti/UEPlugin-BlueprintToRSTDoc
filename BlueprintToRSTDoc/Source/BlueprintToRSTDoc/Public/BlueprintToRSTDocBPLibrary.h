/*!
 * BlueprintToRSTDoc
 *
 * Copyright (c) 2020-2023 nutti
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/ObjectMacros.h"

#include "BlueprintToRSTDocBPLibrary.generated.h"

UENUM()
enum class ERSTDocEdGraphPinKind : uint8
{
	MT_Input,
	MT_Output,
};

USTRUCT()
struct FRSTDocEdGraphPin
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	ERSTDocEdGraphPinKind Kind;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString Type;

	UPROPERTY()
	FString DefaultValue;

	UPROPERTY()
	FString ToolTips;
};

UENUM()
enum class ERSTDocEdGraphKind : uint8
{
	MT_Function,
	MT_Macro,
	MT_Event,
};

USTRUCT()
struct FRSTDocEdGraph
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	ERSTDocEdGraphKind Kind;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	TArray<FString> Category;

	UPROPERTY()
	FString ToolTips;

	UPROPERTY()
	bool bIsConst = false;

	UPROPERTY()
	TArray<FString> AccessModifiers;

	UPROPERTY()
	TArray<FString> Flags;

	UPROPERTY()
	TArray<FRSTDocEdGraphPin> Inputs;

	UPROPERTY()
	TArray<FRSTDocEdGraphPin> Outputs;
};

USTRUCT()
struct FRSTDocProperty
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString Type;

	UPROPERTY()
	TArray<FString> Category;

	UPROPERTY()
	FString ToolTips;

	UPROPERTY()
	TArray<FString> AccessModifiers;

	UPROPERTY()
	TArray<FString> Flags;

	UPROPERTY()
	FString LifetimeCondition;
};

USTRUCT()
struct FRSTDocBlueprint
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FString Path;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString ToolTips;

	UPROPERTY()
	FString ParentName;

	UPROPERTY()
	TArray<FRSTDocEdGraph> Functions;

	UPROPERTY()
	TArray<FRSTDocEdGraph> Macros;

	UPROPERTY()
	TArray<FRSTDocEdGraph> Events;

	UPROPERTY()
	TArray<FRSTDocProperty> Properties;
};

USTRUCT()
struct FRSTDocStructure
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FString Path;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString ToolTips;

	UPROPERTY()
	TArray<FRSTDocProperty> Properties;
};

USTRUCT()
struct FRSTDocEnumerationField
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString ToolTips;

	UPROPERTY()
	int64 Value;
};

USTRUCT()
struct FRSTDocEnumeration
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FString Path;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString ToolTips;

	UPROPERTY()
	TArray<FRSTDocEnumerationField> Fields;
};

UCLASS()
class BLUEPRINTTORSTDOC_API UBlueprintToRSTDocBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BluePrintCallable, exec, category = "BlueprintToRSTDoc")
	static void GenerateRSTDoc(const FString& OutputDirectory, const TArray<FString>& ExcludePaths, bool& bSuccess,
		FString& ErrorMessage, bool bOutputBlueprint = true, bool bOutputStructure = true, bool bOutputEnumeration = true,
		bool bOutputDocsList = true, const FString& OutputDocsListFileName = "DocsList.txt", bool bOutputDocsListFullPath = false);
};