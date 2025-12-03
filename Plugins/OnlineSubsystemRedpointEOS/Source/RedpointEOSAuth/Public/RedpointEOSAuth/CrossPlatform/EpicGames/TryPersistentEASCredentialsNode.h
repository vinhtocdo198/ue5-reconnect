// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/AuthenticationGraphNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1405438762, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;

#define EOS_METADATA_NO_EMPTY_EAS_REFRESH_TOKEN_ATTEMPT TEXT("NoEmptyEASRefreshTokenAttempt")
#define EOS_METADATA_EAS_REFRESH_TOKEN TEXT("EASRefreshToken")
#define EOS_METADATA_EAS_NATIVE_SUBSYSTEM TEXT("EASNativeSubsystem")

class REDPOINTEOSAUTH_API FTryPersistentEASCredentialsNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FTryPersistentEASCredentialsNode);
    FTryPersistentEASCredentialsNode() = default;
    virtual ~FTryPersistentEASCredentialsNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FTryPersistentEASCredentialsNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(1405438762, Redpoint::EOS::Auth::CrossPlatform::EpicGames, FTryPersistentEASCredentialsNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()