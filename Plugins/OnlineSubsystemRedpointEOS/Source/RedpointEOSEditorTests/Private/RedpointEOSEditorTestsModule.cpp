// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditorTestsModule.h"

DEFINE_LOG_CATEGORY(LogRedpointEOSEditorTestsModule);

void FRedpointEOSEditorTestsModule::StartupModule()
{
    UE_LOG(LogRedpointEOSEditorTestsModule, Verbose, TEXT("Module is starting up."));
}

void FRedpointEOSEditorTestsModule::ShutdownModule()
{
    UE_LOG(LogRedpointEOSEditorTestsModule, Verbose, TEXT("Module is shutting down."));
}

IMPLEMENT_MODULE(FRedpointEOSEditorTestsModule, RedpointEOSEditorTests);
