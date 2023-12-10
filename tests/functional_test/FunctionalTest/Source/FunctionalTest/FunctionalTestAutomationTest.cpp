/*!
 * FunctionalTest
 *
 * Copyright (c) 2019-2023 nutti
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "BPLibrary.h"
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
	UBlueprintToRSTDocBPLibrary::GenerateRSTDoc(
		OutputDirectory, {"ChaosNiagara", "Engine", "DatasmithContent", "Niagara"}, bSuccess, ErrorMessage);
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
		return 1;
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
		return 1;
	}

	TMap<FString, FString> FilePairs;
	for (auto E : ExpectedFiles)
	{
		FString FileName = E.Mid(ExpectedFilesDirectory.Len());
		int32 Index = ActualFiles.Find(ActualFilesDirectory + FileName);
		if (Index == INDEX_NONE)
		{
			UE_LOG(LogTemp, Error, TEXT("Not Found: %s"), *FileName);
			return 1;
		}

		FString Expected;
		FString Actual;
		FFileHelper::LoadFileToString(Expected, *E);
		FFileHelper::LoadFileToString(Actual, *ActualFiles[Index]);
		if (Expected != Actual)
		{
			UE_LOG(LogTemp, Error, TEXT("File Content does not match (File: %s)"), *E);
			return 1;
		}
	}

	PlatformFile.DeleteDirectoryRecursively(*OutputDirectory);

	return 0;
}
