// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraphUserInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4030719118, Redpoint::EOS::OnlineSubsystemRedpointEOS::Authentication)
{
using namespace ::Redpoint::EOS::Auth;

class FEOSSubsystemAuthenticationGraphUserInterface : public IAuthenticationGraphUserInterface
{
public:
    FEOSSubsystemAuthenticationGraphUserInterface() = default;
    UE_NONCOPYABLE(FEOSSubsystemAuthenticationGraphUserInterface);
    virtual ~FEOSSubsystemAuthenticationGraphUserInterface() = default;

    virtual EEventResult AddWidgetToViewport(UWorld *InWorld, UUserWidget *InWidget) override;
    virtual EEventResult RemoveWidgetFromViewport(UWorld *InWorld, UUserWidget *InWidget) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Authentication
{
REDPOINT_EOS_FILE_NS_EXPORT(
    4030719118,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::Authentication,
    FEOSSubsystemAuthenticationGraphUserInterface)
}

#endif