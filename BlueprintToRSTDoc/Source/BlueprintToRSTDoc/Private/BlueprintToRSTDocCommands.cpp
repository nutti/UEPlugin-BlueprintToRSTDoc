/*!
 * BlueprintToRSTDoc
 *
 * Copyright (c) 2020-2023 nutti
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "BlueprintToRSTDocCommands.h"

#define LOCTEXT_NAMESPACE "BlueprintToRSTDoc"

void FBlueprintToRSTDocCommands::RegisterCommands()
{
	UI_COMMAND(Action, "Genereate RST Document", "Generate reStructuredText format documents from Blueprints",
		EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
