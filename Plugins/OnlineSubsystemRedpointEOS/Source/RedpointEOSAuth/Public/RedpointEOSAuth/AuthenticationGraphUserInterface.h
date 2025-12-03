// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(804531275, Redpoint::EOS::Auth)
{

class REDPOINTEOSAUTH_API IAuthenticationGraphUserInterface : public TSharedFromThis<IAuthenticationGraphUserInterface>
{
public:
    IAuthenticationGraphUserInterface() = default;
    UE_NONCOPYABLE(IAuthenticationGraphUserInterface);
    virtual ~IAuthenticationGraphUserInterface() = default;

    enum class EEventResult : uint8
    {
        Handled,
        Unhandled,
    };

    virtual EEventResult AddWidgetToViewport(UWorld *InWorld, UUserWidget *InUserWidget) = 0;
    virtual EEventResult RemoveWidgetFromViewport(UWorld *InWorld, UUserWidget *InUserWidget) = 0;
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(804531275, Redpoint::EOS::Auth, IAuthenticationGraphUserInterface)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()