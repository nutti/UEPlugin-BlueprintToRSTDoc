// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BlueprintToRSTDoc.h"

#include "BPLibrary.h"
#include "Commands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "Settings.h"
#include "Style.h"

#define LOCTEXT_NAMESPACE "FBlueprintToRSTDocModule"

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
	MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, Commands,
		FMenuExtensionDelegate::CreateRaw(this, &FBlueprintToRSTDocModule::AddMenuExtension));
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	TSharedPtr<FExtender> ToolBarExtender = MakeShareable(new FExtender());
	ToolBarExtender->AddToolBarExtension("Settings", EExtensionHook::After, Commands,
		FToolBarExtensionDelegate::CreateRaw(this, &FBlueprintToRSTDocModule::AddToolBarExtension));
	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolBarExtender);

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
	UBlueprintToRSTDocBPLibrary::GenerateRSTDoc();
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
