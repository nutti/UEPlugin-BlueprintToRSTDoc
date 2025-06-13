/*!
 * FunctionalTest
 *
 * Copyright (c) 2020-2023 nutti
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "BlueprintToRSTDocBPLibrary.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/FileManagerGeneric.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFuntionalTestSyncVersion, "BlueprintToRSTDoc.FunctionalTest.GenerateRSTDocument",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter);

bool FFuntionalTestSyncVersion::RunTest(const FString& Parameters)
{
	FString ProjectFilePath = FPaths::GetProjectFilePath();
	FString OutputDirectory = FString::Format(TEXT("{0}/_FunctionalTestOutput"), {FPaths::GetPath(ProjectFilePath)});

	// Create an output directory.
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*OutputDirectory))
	{
		PlatformFile.CreateDirectory(*OutputDirectory);
	}

	// Generate RST documents.
	bool bSuccess;
	FString ErrorMessage;
	TArray<FString> ExcludePaths = {
		"ChaosNiagara",
		"Engine",
		"DatasmithContent",
		"Niagara",
		"ChaosVD",
		"Takes",
		"AudioWidgets",
		"ControlRig",
		"ControlRigModules",
		"ControlRigSpline",
	};
	UBlueprintToRSTDocBPLibrary::GenerateRSTDoc(OutputDirectory, ExcludePaths, bSuccess, ErrorMessage);
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
		return false;
	}

	IFileManager& FileManager = FFileManagerGeneric::Get();
	FString ExpectedFilesDirectory = FString::Format(TEXT("{0}/FunctionalTest/Expected"), {FPaths::GetPath(ProjectFilePath)});
	TArray<FString> ExpectedFiles;
	FString ActualFilesDirectory = OutputDirectory;
	TArray<FString> ActualFiles;
	FileManager.FindFilesRecursive(ExpectedFiles, *ExpectedFilesDirectory, TEXT("*.*"), true, false, false);
	FileManager.FindFilesRecursive(ActualFiles, *ActualFilesDirectory, TEXT("*.*"), true, false, false);

	if (ExpectedFiles.Num() != ActualFiles.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Number of Files is not matched: %d (Expected) vs %d (Actual)"), ExpectedFiles.Num(),
			ActualFiles.Num());
		return false;
	}

	TMap<FString, FString> FilePairs;
	for (auto E : ExpectedFiles)
	{
		FString FileName = E.Mid(ExpectedFilesDirectory.Len());
		int32 Index = ActualFiles.Find(ActualFilesDirectory + FileName);
		if (Index == INDEX_NONE)
		{
			UE_LOG(LogTemp, Error, TEXT("Not Found: %s"), *FileName);
			return false;
		}

		FString Expected;
		FString Actual;
		FFileHelper::LoadFileToString(Expected, *E);
		FFileHelper::LoadFileToString(Actual, *ActualFiles[Index]);
		if (Expected != Actual)
		{
			UE_LOG(LogTemp, Error, TEXT("File Content does not match (File: %s)"), *E);
			return false;
		}
	}

	PlatformFile.DeleteDirectoryRecursively(*OutputDirectory);

	return true;
}
