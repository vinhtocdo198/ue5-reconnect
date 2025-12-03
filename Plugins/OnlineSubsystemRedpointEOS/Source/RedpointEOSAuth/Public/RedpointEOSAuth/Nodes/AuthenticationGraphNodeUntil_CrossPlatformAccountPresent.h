// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1513208775, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::Nodes;

class REDPOINTEOSAUTH_API FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent
    : public FAuthenticationGraphNodeUntil
{
private:
    bool AllowFailureFlag;

    static bool Condition(const FAuthenticationGraphState &State);
    virtual bool RequireConditionPass() const override;

public:
    FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent(
        FString InErrorMessage = TEXT(""),
        FString InEditorSignalContext = TEXT(""),
        FString InEditorSignalId = TEXT(""));
    UE_NONCOPYABLE(FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent);
    virtual ~FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent() override = default;

    virtual TSharedRef<FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent> AllowFailure(bool Allow);

    virtual FString GetDebugName() const override;
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1513208775,
    Redpoint::EOS::Auth::Nodes,
    FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()