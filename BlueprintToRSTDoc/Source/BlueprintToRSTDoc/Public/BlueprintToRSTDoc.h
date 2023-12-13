/*!
 * BlueprintToRSTDoc
 *
 * Copyright (c) 2020-2023 nutti
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#pragma once

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Modules/ModuleManager.h"

class FBlueprintToRSTDocModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void CommandExecuted();

private:
	void AddToolBarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);
};