#pragma once

#include "UObject/ObjectMacros.h"
#include "Framework/Commands/Commands.h"

#include "Style.h"


class FBlueprintToRSTDocCommands : public TCommands<FBlueprintToRSTDocCommands>
{
public:

    FBlueprintToRSTDocCommands() : TCommands<FBlueprintToRSTDocCommands>(
            TEXT("BlueprintToRSTDoc"),
            NSLOCTEXT("Contexts", "BlueprintToRSTDoc", "BlueprintToRSTDoc Plugin"),
            NAME_None,
            FBlueprintToRSTDocStyle::GetStyleSetName()
        )
    {
    }

    virtual void RegisterCommands() override;

    TSharedPtr<FUICommandInfo> Action;
};