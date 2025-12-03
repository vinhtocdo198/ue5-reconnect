// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSControlChannel.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseContext.h"
#include "RedpointEOSNetworking/Auth/Beacon/AuthBeaconPhaseContext.h"
#include "RedpointEOSNetworking/Auth/Queues/QueuedEntry.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(367523679, Redpoint::EOS::Networking::Auth::Queues)
{
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Auth::Queues;
using namespace ::Redpoint::EOS::Networking::Auth::Beacon;

class FQueuedBeaconEntry : public IQueuedEntry, public TSharedFromThis<FQueuedBeaconEntry>
{
public:
    FQueuedBeaconEntry(
        const FString &InBeaconType,
        const UE::Online::FAccountId &InIncomingUser,
        URedpointEOSControlChannel *InControlChannel);
    UE_NONCOPYABLE(FQueuedBeaconEntry);
    virtual ~FQueuedBeaconEntry() override = default;

    FString BeaconType;
    UE::Online::FAccountId IncomingUser;
    TSoftObjectPtr<URedpointEOSControlChannel> ControlChannel;
    TSharedPtr<FAuthBeaconPhaseContext> Context;

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
REDPOINT_EOS_FILE_NS_EXPORT(367523679, Redpoint::EOS::Networking::Auth::Queues, FQueuedBeaconEntry);
}

REDPOINT_EOS_CODE_GUARD_END()