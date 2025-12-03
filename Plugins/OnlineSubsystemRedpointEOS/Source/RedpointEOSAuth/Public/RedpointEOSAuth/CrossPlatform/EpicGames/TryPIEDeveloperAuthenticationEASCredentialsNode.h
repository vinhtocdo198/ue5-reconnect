// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryDeveloperAuthenticationEASCredentialsNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1794112417, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

class REDPOINTEOSAUTH_API FTryPIEDeveloperAuthenticationEASCredentialsNode
    : public FTryDeveloperAuthenticationEASCredentialsNode
{
public:
    FTryPIEDeveloperAuthenticationEASCredentialsNode() = default;
    UE_NONCOPYABLE(FTryPIEDeveloperAuthenticationEASCredentialsNode);
    virtual ~FTryPIEDeveloperAuthenticationEASCredentialsNode() override = default;

protected:
    virtual FString GetCredentialName(TSharedRef<FAuthenticationGraphState> State) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FTryPIEDeveloperAuthenticationEASCredentialsNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1794112417,
    Redpoint::EOS::Auth::CrossPlatform::EpicGames,
    FTryPIEDeveloperAuthenticationEASCredentialsNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()