#pragma once

#include "Styling/SlateStyle.h"
#include "UObject/ObjectMacros.h"

class FBlueprintToRSTDocStyle
{
public:
	static void Initialize();

	static void Shutdown();

	static void ReloadTextures();

	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:
	static TSharedRef<FSlateStyleSet> Create();

	static TSharedPtr<FSlateStyleSet> StyleInstance;
};
