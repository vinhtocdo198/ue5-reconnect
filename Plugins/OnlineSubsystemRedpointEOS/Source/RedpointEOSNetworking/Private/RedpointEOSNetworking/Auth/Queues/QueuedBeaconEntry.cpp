// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Queues/QueuedBeaconEntry.h"

#include "Engine/NetworkDelegates.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(367523679, Redpoint::EOS::Networking::Auth::Queues)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;

FQueuedBeaconEntry::FQueuedBeaconEntry(
    const FString &InBeaconType,
    const UE::Online::FAccountId &InIncomingUser,
    URedpointEOSControlChannel *InControlChannel)
    : BeaconType(InBeaconType)
    , IncomingUser(InIncomingUser)
    , ControlChannel(InControlChannel)
    , Context(nullptr)
{
}

void FQueuedBeaconEntry::SetContext(const TSharedRef<IAuthPhaseContext> &InContext)
{
    this->Context = StaticCastSharedRef<FAuthBeaconPhaseContext>(InContext);
}

void FQueuedBeaconEntry::SendSuccess()
{
    if (!ControlChannel.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("Control channel is no longer valid, not sending NMT_BeaconJoin packet."));
        return;
    }

    // @note: This must remain compatible with the NMT_BeaconJoin definition declared in DataChannel.h!
    auto ReplId = GetUniqueNetIdRepl(this->IncomingUser);
    FOutBunch Bunch;
    Bunch << BeaconType;
    Bunch << ReplId;
    FInBunch InBunch(ControlChannel->Connection, Bunch.GetData(), Bunch.GetNumBits());
    ControlChannel->Connection->Driver->Notify->NotifyControlMessage(
        ControlChannel->Connection,
        NMT_BeaconJoin,
        InBunch);
}

void FQueuedBeaconEntry::SendFailure(const FString &ErrorMessage)
{
    if (!ControlChannel.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("Control channel is no longer valid, not sending NMT_Failure packet."));
        return;
    }

    FString ErrorMessageTemp = ErrorMessage;
    while (ErrorMessageTemp.RemoveFromEnd(TEXT(".")))
        ;
    FString UserErrorMessage = FString::Printf(
        TEXT("%s. You have been disconnected."),
        ErrorMessageTemp.IsEmpty() ? TEXT("Failed to verify your account or EAC integrity on connection")
                                   : *ErrorMessageTemp);
    FNetControlMessage<NMT_Failure>::Send(ControlChannel->Connection, UserErrorMessage);
    ControlChannel->Connection->FlushNet(true);
    ControlChannel->Connection->Close();
}

bool FQueuedBeaconEntry::Contains()
{
    if (ControlChannel->QueuedBeacons.Contains(IncomingUser))
    {
        return ControlChannel->QueuedBeacons[IncomingUser].Contains(BeaconType);
    }
    return false;
}

void FQueuedBeaconEntry::Track()
{
    if (!ControlChannel->QueuedBeacons.Contains(IncomingUser))
    {
        ControlChannel->QueuedBeacons.Add(IncomingUser, TMap<FString, TSharedPtr<FQueuedBeaconEntry>>());
    }
    ControlChannel->QueuedBeacons[IncomingUser].Add(BeaconType, this->AsShared());
}

void FQueuedBeaconEntry::Release()
{
    if (!ControlChannel->QueuedBeacons.Contains(IncomingUser))
    {
        return;
    }
    ControlChannel->QueuedBeacons[IncomingUser].Remove(BeaconType);
}

}

REDPOINT_EOS_CODE_GUARD_END()