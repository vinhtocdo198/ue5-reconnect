// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraphUserInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1780445701, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Auth;

class FNullAuthenticationGraphUserInterface : public IAuthenticationGraphUserInterface
{
public:
    FNullAuthenticationGraphUserInterface() = default;
    UE_NONCOPYABLE(FNullAuthenticationGraphUserInterface);
    virtual ~FNullAuthenticationGraphUserInterface() override = default;

    virtual EEventResult AddWidgetToViewport(UWorld *InWorld, UUserWidget *InWidget) override;
    virtual EEventResult RemoveWidgetFromViewport(UWorld *InWorld, UUserWidget *InWidget) override;
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(1780445701, Redpoint::EOS::Identity, FNullAuthenticationGraphUserInterface)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()