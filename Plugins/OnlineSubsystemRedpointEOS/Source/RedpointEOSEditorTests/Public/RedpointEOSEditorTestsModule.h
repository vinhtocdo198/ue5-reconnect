// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRedpointEOSEditorTestsModule, All, All);

class FRedpointEOSEditorTestsModule : public FDefaultModuleImpl
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};