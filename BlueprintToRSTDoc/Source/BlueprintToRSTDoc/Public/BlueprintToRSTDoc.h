// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"


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