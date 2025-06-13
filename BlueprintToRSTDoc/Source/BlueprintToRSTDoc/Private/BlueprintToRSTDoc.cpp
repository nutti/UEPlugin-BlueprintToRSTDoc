/*!
 * BlueprintToRSTDoc
 *
 * Copyright (c) 2020-2023 nutti
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "BlueprintToRSTDoc.h"

#include "BlueprintToRSTDocBPLibrary.h"
#include "BlueprintToRSTDocCommands.h"
#include "BlueprintToRSTDocCommon.h"
#include "BlueprintToRSTDocSettings.h"
#include "BlueprintToRSTDocStyle.h"
#include "DesktopPlatformModule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IDesktopPlatform.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "Misc/EngineVersionComparison.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "BlueprintToRSTDoc"

bool OpenOutputDirectory(FString& OutDirectory)
{
	void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		ERROR_MESSAGE_BOX(TEXT("Failed to get DesktopPlatform.\n"));
		return false;
	}
	if (!DesktopPlatform->OpenDirectoryDialog(ParentWindowPtr, TEXT("Save RST Documents To"), TEXT(""), OutDirectory))
	{
		return false;
	}

	return true;
}

void FBlueprintToRSTDocModule::StartupModule()
{
	FBlueprintToRSTDocStyle::Initialize();
	FBlueprintToRSTDocStyle::ReloadTextures();

	FBlueprintToRSTDocCommands::Register();

	TSharedPtr<class FUICommandList> Commands = MakeShareable(new FUICommandList);
	Commands->MapAction(FBlueprintToRSTDocCommands::Get().Action,
		FExecuteAction::CreateRaw(this, &FBlueprintToRSTDocModule::CommandExecuted), FCanExecuteAction());

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuExtension("Tools", EExtensionHook::After, Commands,
		FMenuExtensionDelegate::CreateRaw(this, &FBlueprintToRSTDocModule::AddMenuExtension));
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

#if !UE_VERSION_OLDER_THAN(5, 0, 0)
	UToolMenu* UserToolMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	FToolMenuSection& UserToolMenuSection = UserToolMenu->AddSection("BlueprintToRSTDoc");
	FToolMenuEntry& Entry =
		UserToolMenuSection.AddEntry(FToolMenuEntry::InitToolBarButton(FBlueprintToRSTDocCommands::Get().Action));
	Entry.SetCommandList(Commands);
#else
	TSharedPtr<FExtender> ToolBarExtender = MakeShareable(new FExtender());
	ToolBarExtender->AddToolBarExtension("LevelEditor.LevelEditorToolBar.User", EExtensionHook::After, Commands,
		FToolBarExtensionDelegate::CreateRaw(this, &FBlueprintToRSTDocModule::AddToolBarExtension));
	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolBarExtender);
#endif

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	UBlueprintToRSTDocSettings* Settings = GetMutableDefault<UBlueprintToRSTDocSettings>();
	if (SettingsModule)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "BlueprintToRSTDoc",
			LOCTEXT("RuntimeSettingsName", "BlueprintToRSTDoc"), LOCTEXT("RuntimeSettingsDescription", "BlueprintToRSTDoc"),
			Settings);
	}
}

void FBlueprintToRSTDocModule::ShutdownModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "BlueprintToRSTDoc");
	}

	FBlueprintToRSTDocCommands::Unregister();
	FBlueprintToRSTDocStyle::Shutdown();
}

void FBlueprintToRSTDocModule::CommandExecuted()
{
	UBlueprintToRSTDocSettings* Settings = GetMutableDefault<UBlueprintToRSTDocSettings>();

	FString OutputDirectory = Settings->OutputDirectory;
	if (Settings->bAlwaysAskOutputDirectory)
	{
		if (!OpenOutputDirectory(OutputDirectory))
		{
			return;
		}
	}

	bool bSuccess;
	FString ErrorMessage;
	UBlueprintToRSTDocBPLibrary::GenerateRSTDoc(OutputDirectory, Settings->ExcludePaths, bSuccess, ErrorMessage,
		Settings->bOutputBlueprint, Settings->bOutputStructure, Settings->bOutputEnumeration, Settings->bOutputDocsList,
		Settings->OutputDocsListFileName, Settings->bOutputDocsListFullPath);

	if (!bSuccess)
	{
		ERROR_MESSAGE_BOX(ErrorMessage);
	}
	else
	{
		FPlatformMisc::MessageBoxExt(EAppMsgType::Ok, TEXT("Generated documents successfully."), TEXT("BlueprintToRSTDoc"));
	}
}

void FBlueprintToRSTDocModule::AddToolBarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FBlueprintToRSTDocCommands::Get().Action);
}

void FBlueprintToRSTDocModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FBlueprintToRSTDocCommands::Get().Action);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBlueprintToRSTDocModule, BlueprintToRSTDoc)
