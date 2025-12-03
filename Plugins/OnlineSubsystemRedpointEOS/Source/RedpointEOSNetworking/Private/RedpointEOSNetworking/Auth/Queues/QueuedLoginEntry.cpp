// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Queues/QueuedLoginEntry.h"

#include "Engine/NetworkDelegates.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Auth/Login/AuthLoginPhaseContext.h"
#include "RedpointEOSNetworking/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1850804168, Redpoint::EOS::Networking::Auth::Queues)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;

FQueuedLoginEntry::FQueuedLoginEntry(
    const FString &InClientResponse,
    const FString &InURLString,
    const UE::Online::FAccountId &InIncomingUser,
    const FString &InOnlinePlatformNameString,
    URedpointEOSControlChannel *InControlChannel)
    : ClientResponse(InClientResponse)
    , URLString(InURLString)
    , IncomingUser(InIncomingUser)
    , OnlinePlatformNameString(InOnlinePlatformNameString)
    , ControlChannel(InControlChannel)
    , Context(nullptr)
{
}

void FQueuedLoginEntry::SetContext(const TSharedRef<IAuthPhaseContext> &InContext)
{
    this->Context = StaticCastSharedRef<FAuthLoginPhaseContext>(InContext);
}

void FQueuedLoginEntry::SendSuccess()
{
    if (!ControlChannel.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworking,
            Error,
            TEXT("Control channel is no longer valid, not sending NMT_Login packet."));
        return;
    }

    // @note: This must remain compatible with the NMT_Login definition declared in DataChannel.h!
    auto ReplId = GetUniqueNetIdRepl(this->IncomingUser);
    FOutBunch Bunch;
    Bunch << ClientResponse;
    Bunch << URLString;
    Bunch << ReplId;
    Bunch << OnlinePlatformNameString;
    FInBunch InBunch(ControlChannel->Connection, Bunch.GetData(), Bunch.GetNumBits());
    ControlChannel->Connection->Driver->Notify->NotifyControlMessage(ControlChannel->Connection, NMT_Login, InBunch);
}

void FQueuedLoginEntry::SendFailure(const FString &ErrorMessage)
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

bool FQueuedLoginEntry::Contains()
{
    return ControlChannel->QueuedLogins.Contains(IncomingUser);
}

void FQueuedLoginEntry::Track()
{
    ControlChannel->QueuedLogins.Add(IncomingUser, this->AsShared());
}

void FQueuedLoginEntry::Release()
{
    ControlChannel->QueuedLogins.Remove(IncomingUser);
}

}

REDPOINT_EOS_CODE_GUARD_END()