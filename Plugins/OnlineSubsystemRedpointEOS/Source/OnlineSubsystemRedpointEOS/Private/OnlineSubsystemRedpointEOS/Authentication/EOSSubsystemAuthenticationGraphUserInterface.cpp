// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Authentication/EOSSubsystemAuthenticationGraphUserInterface.h"

#include "EOSSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4030719118, Redpoint::EOS::OnlineSubsystemRedpointEOS::Authentication)
{
using namespace ::Redpoint::EOS::Auth;

IAuthenticationGraphUserInterface::EEventResult FEOSSubsystemAuthenticationGraphUserInterface::AddWidgetToViewport(
    UWorld *InWorld,
    UUserWidget *InWidget)
{
    UEOSSubsystem *GlobalSubsystem = UEOSSubsystem::GetSubsystem(InWorld);
    if (IsValid(GlobalSubsystem))
    {
        if (GlobalSubsystem->OnAddWidgetToViewport.IsBound())
        {
            // Defer to the event instead.
            GlobalSubsystem->OnAddWidgetToViewport.Broadcast(InWidget);
            return EEventResult::Handled;
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOS,
            Warning,
            TEXT("Could not get reference to UEOSSubsystem. Widgets will be directly added to the viewport."));
    }
    return EEventResult::Unhandled;
}

IAuthenticationGraphUserInterface::EEventResult FEOSSubsystemAuthenticationGraphUserInterface::RemoveWidgetFromViewport(
    UWorld *InWorld,
    UUserWidget *InWidget)
{
    UEOSSubsystem *GlobalSubsystem = UEOSSubsystem::GetSubsystem(InWorld);
    if (IsValid(GlobalSubsystem))
    {
        if (GlobalSubsystem->OnRemoveWidgetFromViewport.IsBound())
        {
            // Defer to the event instead.
            GlobalSubsystem->OnRemoveWidgetFromViewport.Broadcast(InWidget);
            return EEventResult::Handled;
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOS,
            Warning,
            TEXT("Could not get reference to UEOSSubsystem. Widgets will be directly removed from the viewport."));
    }
    return EEventResult::Unhandled;
}

}

#endif