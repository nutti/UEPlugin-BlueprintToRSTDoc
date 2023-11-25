#include "Commands.h"

#define LOCTEXT_NAMESPACE "FBlueprintToRSTDocModule"

void FBlueprintToRSTDocCommands::RegisterCommands()
{
	UI_COMMAND(Action, "BlueprintToRSTDoc", "Execute BlueprintToRSTDoc action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
