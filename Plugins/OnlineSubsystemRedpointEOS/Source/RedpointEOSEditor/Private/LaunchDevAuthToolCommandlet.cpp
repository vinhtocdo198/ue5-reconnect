// Copyright June Rhodes. All Rights Reserved.

#include "LaunchDevAuthToolCommandlet.h"

#include "RedpointEOSEditor/DevAuthToolLauncher.h"
#include "RedpointEOSEditorModule.h"

ULaunchDevAuthToolCommandlet::ULaunchDevAuthToolCommandlet(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    IsClient = false;
    IsServer = false;
    IsEditor = true;

    LogToConsole = true;
    ShowErrorCount = true;
    ShowProgress = true;
}

int32 ULaunchDevAuthToolCommandlet::Main(const FString &Params)
{
    using namespace ::Redpoint::EOS::Editor;

#if PLATFORM_WINDOWS || PLATFORM_MAC
    UE_LOG(LogRedpointEOSEditor, Verbose, TEXT("EOS DevAuthTool Commandlet Started"));
    return FDevAuthToolLauncher::Get().Launch(false, true) ? 0 : 1;
#else
    UE_LOG(
        LogRedpointEOSEditor,
        Error,
        TEXT("The Redpoint EOS plugin can not automatically launch the Developer Authentication Tool on this platform "
             "for you. Please launch it manually from the EOS SDK."));
    return 1;
#endif
}