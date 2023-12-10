#include "Style.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

#define LOCTEXT_NAMESPACE "BlueprintToRSTDoc"

TSharedPtr<FSlateStyleSet> FBlueprintToRSTDocStyle::StyleInstance = nullptr;

void FBlueprintToRSTDocStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FBlueprintToRSTDocStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	check(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

void FBlueprintToRSTDocStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FBlueprintToRSTDocStyle::Get()
{
	return *StyleInstance;
}

FName FBlueprintToRSTDocStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("BlueprintToRSTDocStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FBlueprintToRSTDocStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("BlueprintToRSTDoc")->GetBaseDir() / TEXT("Resources"));
	Style->Set("BlueprintToRSTDoc.Action",
		new FSlateImageBrush(Style->RootToContentDir(TEXT("Button_40x"), TEXT(".png")), FVector2D(40.0f, 40.0f)));
	Style->Set("BlueprintToRSTDoc.Action.Small",
		new FSlateImageBrush(Style->RootToContentDir(TEXT("Button_40x"), TEXT(".png")), FVector2D(20.0f, 20.0f)));

	return Style;
}

#undef LOCTEXT_NAMESPACE
