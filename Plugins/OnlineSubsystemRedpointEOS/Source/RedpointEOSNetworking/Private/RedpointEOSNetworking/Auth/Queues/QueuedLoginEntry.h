// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSControlChannel.h"
#include "RedpointEOSNetworking/Auth/Login/AuthLoginPhaseContext.h"
#include "RedpointEOSNetworking/Auth/Queues/QueuedEntry.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1850804168, Redpoint::EOS::Networking::Auth::Queues)
{
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Auth::Queues;
using namespace ::Redpoint::EOS::Networking::Auth::Login;

class FQueuedLoginEntry : public IQueuedEntry, public TSharedFromThis<FQueuedLoginEntry>
{
public:
    FQueuedLoginEntry(
        const FString &InClientResponse,
        const FString &InURLString,
        const UE::Online::FAccountId &InIncomingUser,
        const FString &InOnlinePlatformNameString,
        URedpointEOSControlChannel *InControlChannel);
    UE_NONCOPYABLE(FQueuedLoginEntry);
    virtual ~FQueuedLoginEntry() override = default;

    FString ClientResponse;
    FString URLString;
    UE::Online::FAccountId IncomingUser;
    FString OnlinePlatformNameString;
    TSoftObjectPtr<URedpointEOSControlChannel> ControlChannel;
    TSharedPtr<FAuthLoginPhaseContext> Context;

    virtual void SetContext(const TSharedRef<IAuthPhaseContext> &InContext) override;
    virtual void SendSuccess() override;
    virtual void SendFailure(const FString &Message) override;
    virtual bool Contains() override;
    virtual void Track() override;
    virtual void Release() override;
};

}

namespace Redpoint::EOS::Networking::Auth::Queues
{
REDPOINT_EOS_FILE_NS_EXPORT(1850804168, Redpoint::EOS::Networking::Auth::Queues, FQueuedLoginEntry);
}

REDPOINT_EOS_CODE_GUARD_END()