// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

#include "Interfaces/OnlineIdentityInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3400370413, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FLogUserIntoLocalPlatformNode : public FAuthenticationGraphNode
{
private:
    void OnOSSLoginUIClosed(
        TSharedPtr<const FUniqueNetId> UniqueId,
        const int ControllerIndex,
        const FOnlineError &Error,
        TSharedRef<FAuthenticationGraphState> InState,
        FAuthenticationGraphNodeOnDone InOnDone,
        TSharedPtr<IOnlineIdentity, ESPMode::ThreadSafe> OSSIdentity);

    void DoLogin(
        const TSharedRef<FAuthenticationGraphState> &InState,
        const FAuthenticationGraphNodeOnDone &InOnDone,
        const TSharedPtr<IOnlineIdentity, ESPMode::ThreadSafe> &OSSIdentity);
    FDelegateHandle OnOSSLoginDelegate;
    void OnOSSLoginComplete(
        int32 LocalUserNum,
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &Error,
        TSharedRef<FAuthenticationGraphState> InState,
        FAuthenticationGraphNodeOnDone InOnDone,
        TSharedPtr<IOnlineIdentity, ESPMode::ThreadSafe> OSSIdentity);

public:
    FLogUserIntoLocalPlatformNode() = default;
    UE_NONCOPYABLE(FLogUserIntoLocalPlatformNode);
    virtual ~FLogUserIntoLocalPlatformNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FLogUserIntoLocalPlatformNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(3400370413, Redpoint::EOS::Auth::Nodes, FLogUserIntoLocalPlatformNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()