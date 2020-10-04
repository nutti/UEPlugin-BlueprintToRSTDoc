#pragma once

#include "UObject/ObjectMacros.h"
#include "Styling/SlateStyle.h"

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
