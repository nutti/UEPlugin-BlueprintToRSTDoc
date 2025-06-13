/*!
 * BlueprintToRSTDoc
 *
 * Copyright (c) 2020-2023 nutti
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include "BlueprintToRSTDocStyle.h"
#include "Framework/Commands/Commands.h"
#include "UObject/ObjectMacros.h"

class FBlueprintToRSTDocCommands : public TCommands<FBlueprintToRSTDocCommands>
{
public:
	FBlueprintToRSTDocCommands()
		: TCommands<FBlueprintToRSTDocCommands>(TEXT("BlueprintToRSTDoc"),
			  NSLOCTEXT("Contexts", "BlueprintToRSTDoc", "BlueprintToRSTDoc Plugin"), NAME_None,
			  FBlueprintToRSTDocStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> Action;
};