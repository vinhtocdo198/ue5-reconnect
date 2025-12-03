// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2014916715, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::Nodes;

class REDPOINTEOSAUTH_API FAuthenticationGraphNodeUntil_LoginComplete : public FAuthenticationGraphNodeUntil
{
private:
    static bool Condition(const FAuthenticationGraphState &State);

public:
    FAuthenticationGraphNodeUntil_LoginComplete(FString InErrorMessage = TEXT(""));
    UE_NONCOPYABLE(FAuthenticationGraphNodeUntil_LoginComplete);
    virtual ~FAuthenticationGraphNodeUntil_LoginComplete() override = default;

    virtual FString GetDebugName() const override;
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(2014916715, Redpoint::EOS::Auth::Nodes, FAuthenticationGraphNodeUntil_LoginComplete)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()