#include "Commands.h"

#define LOCTEXT_NAMESPACE "BlueprintToRSTDoc"

void FBlueprintToRSTDocCommands::RegisterCommands()
{
	UI_COMMAND(Action, "Genereate RST Document", "Generate reStructuredText format documents from Blueprints",
		EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
