// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemEOSEditorCommands.h"

#define LOCTEXT_NAMESPACE "FOnlineSubsystemRedpointEOSEditorModule"

void FOnlineSubsystemEOSEditorCommands::RegisterCommands()
{
    UI_COMMAND(
        ViewDocumentation,
        "View Documentation",
        "View documentation for the plugin.",
        EUserInterfaceActionType::Button,
        FInputChord());
#if REDPOINT_EOS_IS_FREE_EDITION
    UI_COMMAND(
        ViewLicenseAgreementInBrowser,
        "View License Agreement",
        "View the license agreement that applies to EOS Online Framework (Free Edition).",
        EUserInterfaceActionType::Button,
        FInputChord());
    UI_COMMAND(
        UpgradeToPaid,
        "Upgrade to Paid Edition",
        "Upgrade to the Paid Edition of EOS Online Framework, and get access to support, source code and more "
        "features!",
        EUserInterfaceActionType::Button,
        FInputChord());
#else
    UI_COMMAND(
        AccessSupport,
        "Access Support",
        "Find out how to access support for the plugin.",
        EUserInterfaceActionType::Button,
        FInputChord());
#endif
    UI_COMMAND(
        ViewWebsite,
        "Open Website",
        "Open the website for EOS Online Framework.",
        EUserInterfaceActionType::Button,
        FInputChord());
}

#undef LOCTEXT_NAMESPACE