// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSessionSettings.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(92848785, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session)
{
using namespace ::Redpoint::EOS::API;

class FOnlineSessionSearchResultEOS : public FOnlineSessionSearchResult
{
public:
    static FOnlineSessionSearchResultEOS CreateInvalid();
    static FOnlineSessionSearchResultEOS CreateFromDetails(
        const FPlatformHandle &InPlatformHandle,
        EOS_HSessionDetails InHandle);
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session
{
REDPOINT_EOS_FILE_NS_EXPORT(
    92848785,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session,
    FOnlineSessionSearchResultEOS)
}

REDPOINT_EOS_CODE_GUARD_END()