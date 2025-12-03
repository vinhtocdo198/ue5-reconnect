// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3315570403, Redpoint::EOS::Networking::Auth::Queues)
{
using namespace ::Redpoint::EOS::Networking::Auth;

class IQueuedEntry
{
public:
    virtual ~IQueuedEntry() = default;
    virtual void SetContext(const TSharedRef<IAuthPhaseContext> &InContext) = 0;
    virtual void SendSuccess() = 0;
    virtual void SendFailure(const FString &Message) = 0;
    virtual bool Contains() = 0;
    virtual void Track() = 0;
    virtual void Release() = 0;
};

}

namespace Redpoint::EOS::Networking::Auth::Queues
{
REDPOINT_EOS_FILE_NS_EXPORT(3315570403, Redpoint::EOS::Networking::Auth::Queues, IQueuedEntry);
}

REDPOINT_EOS_CODE_GUARD_END()