// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryDeveloperAuthenticationEASCredentialsNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(699046853, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

class REDPOINTEOSAUTH_API FTryDefaultDeveloperAuthenticationEASCredentialsNode
    : public FTryDeveloperAuthenticationEASCredentialsNode
{
public:
    FTryDefaultDeveloperAuthenticationEASCredentialsNode() = default;
    UE_NONCOPYABLE(FTryDefaultDeveloperAuthenticationEASCredentialsNode);
    virtual ~FTryDefaultDeveloperAuthenticationEASCredentialsNode() override = default;

protected:
    virtual FString GetCredentialName(TSharedRef<FAuthenticationGraphState> State) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FTryDefaultDeveloperAuthenticationEASCredentialsNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(
    699046853,
    Redpoint::EOS::Auth::CrossPlatform::EpicGames,
    FTryDefaultDeveloperAuthenticationEASCredentialsNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()