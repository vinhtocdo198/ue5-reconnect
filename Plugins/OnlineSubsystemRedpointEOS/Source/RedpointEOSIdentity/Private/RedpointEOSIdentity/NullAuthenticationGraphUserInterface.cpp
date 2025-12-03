// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/NullAuthenticationGraphUserInterface.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1780445701, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Auth;

IAuthenticationGraphUserInterface::EEventResult FNullAuthenticationGraphUserInterface::AddWidgetToViewport(
    UWorld *InWorld,
    UUserWidget *InWidget)
{
    return EEventResult::Unhandled;
}

IAuthenticationGraphUserInterface::EEventResult FNullAuthenticationGraphUserInterface::RemoveWidgetFromViewport(
    UWorld *InWorld,
    UUserWidget *InWidget)
{
    return EEventResult::Unhandled;
}

}

#endif