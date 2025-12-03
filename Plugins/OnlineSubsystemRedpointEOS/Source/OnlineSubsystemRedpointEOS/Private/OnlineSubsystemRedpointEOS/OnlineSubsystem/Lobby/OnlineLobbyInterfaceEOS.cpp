// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Lobby/OnlineLobbyInterfaceEOS.h"

#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Lobby/OnlineLobbyEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Lobby/OnlineLobbyMemberTransactionEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Lobby/OnlineLobbyTransactionEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointBase.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/SubsystemNames.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManager.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManagerLocalUser.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeatureRequest.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2303545192, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::Error;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::API;

FOnlineLobbyInterfaceEOS::FOnlineLobbyInterfaceEOS(
    const TSharedRef<IRoomSystem> &InRoomSystem,
    const TSharedRef<class FOnlineSubsystemEOS, ESPMode::ThreadSafe> &InSubsystem,
    const TSharedRef<FEOSVoiceManager> &InVoiceManager)
    : RoomSystem(InRoomSystem)
    , EOSSubsystem(InSubsystem)
    , VoiceManager(InVoiceManager)
{
}

void FOnlineLobbyInterfaceEOS::RegisterEvents()
{
    this->OnRoomUpdatedHandle =
        this->RoomSystem->OnRoomUpdated(NAME_Game).AddSP(this, &FOnlineLobbyInterfaceEOS::OnRoomUpdated);
    this->OnRoomLeftHandle = this->RoomSystem->OnRoomLeft(NAME_Game).AddSP(this, &FOnlineLobbyInterfaceEOS::OnRoomLeft);
}

FOnlineLobbyInterfaceEOS::~FOnlineLobbyInterfaceEOS()
{
    this->RoomSystem->OnRoomUpdated(NAME_Game).Remove(this->OnRoomUpdatedHandle);
    this->RoomSystem->OnRoomLeft(NAME_Game).Remove(this->OnRoomLeftHandle);
    for (const auto &KV : this->OnRoomMemberJoinedHandles)
    {
        KV.Key->OnRoomMemberJoined().Remove(KV.Value);
    }
    for (const auto &KV : this->OnRoomMemberUpdatedHandles)
    {
        KV.Key->OnRoomMemberUpdated().Remove(KV.Value);
    }
    for (const auto &KV : this->OnRoomMemberPromotedHandles)
    {
        KV.Key->OnRoomMemberPromoted().Remove(KV.Value);
    }
    for (const auto &KV : this->OnRoomMemberLeftHandles)
    {
        KV.Key->OnRoomMemberLeft().Remove(KV.Value);
    }
    this->OnRoomUpdatedHandle.Reset();
    this->OnRoomLeftHandle.Reset();
    this->OnRoomMemberJoinedHandles.Reset();
    this->OnRoomMemberUpdatedHandles.Reset();
    this->OnRoomMemberLeftHandles.Reset();
}

void FOnlineLobbyInterfaceEOS::RegisterEventsForNewRoom(FRoomRef Room)
{
    this->OnRoomMemberJoinedHandles.Add(
        Room,
        Room->OnRoomMemberJoined().AddSP(this, &FOnlineLobbyInterfaceEOS::OnRoomMemberJoined));
    this->OnRoomMemberUpdatedHandles.Add(
        Room,
        Room->OnRoomMemberUpdated().AddSP(this, &FOnlineLobbyInterfaceEOS::OnRoomMemberUpdated));
    this->OnRoomMemberPromotedHandles.Add(
        Room,
        Room->OnRoomMemberPromoted().AddSP(this, &FOnlineLobbyInterfaceEOS::OnRoomMemberPromoted));
    this->OnRoomMemberLeftHandles.Add(
        Room,
        Room->OnRoomMemberLeft().AddSP(this, &FOnlineLobbyInterfaceEOS::OnRoomMemberLeft));
}

void FOnlineLobbyInterfaceEOS::OnRoomUpdated(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    const FRoomUpdateInfo &RoomUpdateInfo)
{
    if (!this->OnRoomMemberJoinedHandles.Contains(Room))
    {
        // We're only interested in rooms created through IOnlineLobby.
        return;
    }

    this->TriggerOnLobbyUpdateDelegates(
        *GetUniqueNetId(EventContext.LocalUserId),
        *FOnlinePartyIdRedpointBase::Create(Room->GetRoomId()));
}

void FOnlineLobbyInterfaceEOS::OnRoomLeft(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    ERoomLeaveReason Reason)
{
    if (!this->OnRoomMemberJoinedHandles.Contains(Room))
    {
        // We're only interested in rooms created through IOnlineLobby.
        return;
    }

    Room->OnRoomMemberJoined().Remove(this->OnRoomMemberJoinedHandles[Room]);
    Room->OnRoomMemberUpdated().Remove(this->OnRoomMemberUpdatedHandles[Room]);
    Room->OnRoomMemberPromoted().Remove(this->OnRoomMemberPromotedHandles[Room]);
    Room->OnRoomMemberLeft().Remove(this->OnRoomMemberLeftHandles[Room]);
    this->OnRoomMemberJoinedHandles.Remove(Room);
    this->OnRoomMemberUpdatedHandles.Remove(Room);
    this->OnRoomMemberPromotedHandles.Remove(Room);
    this->OnRoomMemberLeftHandles.Remove(Room);

    this->TriggerOnMemberDisconnectDelegates(
        *GetUniqueNetId(EventContext.LocalUserId),
        *FOnlinePartyIdRedpointBase::Create(Room->GetRoomId()),
        *GetUniqueNetId(EventContext.LocalUserId),
        Reason == ERoomLeaveReason::KickedFromRoom);
    this->TriggerOnLobbyDeleteDelegates(
        *GetUniqueNetId(EventContext.LocalUserId),
        *FOnlinePartyIdRedpointBase::Create(Room->GetRoomId()));
}

void FOnlineLobbyInterfaceEOS::OnRoomMemberJoined(
    const FRoomEventContext &EventContext,
    const TSharedRef<IRoom> &Room,
    FRoomUserId TargetUser)
{
    this->TriggerOnMemberConnectDelegates(
        *GetUniqueNetId(EventContext.LocalUserId),
        *FOnlinePartyIdRedpointBase::Create(Room->GetRoomId()),
        *GetUniqueNetId(TargetUser));
}

void FOnlineLobbyInterfaceEOS::OnRoomMemberUpdated(
    const FRoomEventContext &EventContext,
    const TSharedRef<IRoom> &Room,
    FRoomUserId TargetUser,
    const FRoomMemberUpdateInfo &RoomMemberUpdateInfo)
{
    this->TriggerOnMemberUpdateDelegates(
        *GetUniqueNetId(EventContext.LocalUserId),
        *FOnlinePartyIdRedpointBase::Create(Room->GetRoomId()),
        *GetUniqueNetId(TargetUser));
}

void FOnlineLobbyInterfaceEOS::OnRoomMemberPromoted(
    const FRoomEventContext &EventContext,
    const TSharedRef<IRoom> &Room,
    FRoomUserId TargetUser)
{
    this->TriggerOnMemberUpdateDelegates(
        *GetUniqueNetId(EventContext.LocalUserId),
        *FOnlinePartyIdRedpointBase::Create(Room->GetRoomId()),
        *GetUniqueNetId(TargetUser));
}

void FOnlineLobbyInterfaceEOS::OnRoomMemberLeft(
    const FRoomEventContext &EventContext,
    const TSharedRef<IRoom> &Room,
    FRoomUserId TargetUser,
    ERoomLeaveReason LeaveReason)
{
    // @note: We can't see bWasKicked from OnRoomMemberLeft, so we trigger
    // the OnMemberDisconnect for the local user as part of OnRoomLeft instead.
    if (EventContext.LocalUserId != TargetUser)
    {
        this->TriggerOnMemberDisconnectDelegates(
            *GetUniqueNetId(EventContext.LocalUserId),
            *FOnlinePartyIdRedpointBase::Create(Room->GetRoomId()),
            *GetUniqueNetId(TargetUser),
            false);
    }
}

FRoomPtr FOnlineLobbyInterfaceEOS::GetCurrentRoomByPredicate(
    FRoomUserId UserId,
    TFunction<bool(const FRoomRef &Room)> Predicate)
{
    TArray<FRoomRef> CurrentRooms =
        this->RoomSystem->GetCurrentRooms(FGetCurrentRoomsRequest(ERoomScope::User, UserId, NAME_Game));
    FRoomRef *RoomPtr = CurrentRooms.FindByPredicate(Predicate);
    if (RoomPtr == nullptr)
    {
        return nullptr;
    }
    return *RoomPtr;
}

FDateTime FOnlineLobbyInterfaceEOS::GetUtcNow()
{
    return FDateTime::UtcNow();
}

TSharedPtr<FOnlineLobbyTransaction> FOnlineLobbyInterfaceEOS::MakeCreateLobbyTransaction(const FUniqueNetId &UserId)
{
    return MakeShared<FOnlineLobbyTransactionEOS>();
}

TSharedPtr<FOnlineLobbyTransaction> FOnlineLobbyInterfaceEOS::MakeUpdateLobbyTransaction(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId)
{
    return MakeShared<FOnlineLobbyTransactionEOS>();
}

TSharedPtr<FOnlineLobbyMemberTransaction> FOnlineLobbyInterfaceEOS::MakeUpdateLobbyMemberTransaction(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId)
{
    return MakeShared<FOnlineLobbyMemberTransactionEOS>();
}

bool FOnlineLobbyInterfaceEOS::CreateLobby(
    const FUniqueNetId &UserId,
    const FOnlineLobbyTransaction &Transaction,
    FOnLobbyCreateOrConnectComplete OnComplete)
{
    // Verify we have an EOS user as the actor.
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        OnComplete.ExecuteIfBound(AsOSSError(Errors::InvalidUser()), UserId, nullptr);
        return true;
    }

    // Create the request.
    FCreateRoomRequest CreateRoomRequest = FCreateRoomRequest(
        ERoomScope::User,
        GetAccountId(UserId),
        FName(TEXT("Lobby")),
        NAME_Game,
        Transaction.Capacity.Get(EOS_LOBBY_MAX_LOBBY_MEMBERS));

    // Set permission level to either private or public.
    if (Transaction.Public.IsSet() && Transaction.Public.GetValue())
    {
        CreateRoomRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
    }

    // Set the bucket ID, or leave it as "Default".
    {
        FString BucketId = TEXT("Default");
        if (Transaction.SetMetadata.Contains(EOS_LOBBY_SEARCH_BUCKET_ID) &&
            Transaction.SetMetadata[EOS_LOBBY_SEARCH_BUCKET_ID].GetType() == EOnlineKeyValuePairDataType::String)
        {
            Transaction.SetMetadata[EOS_LOBBY_SEARCH_BUCKET_ID].GetValue(BucketId);
        }
        CreateRoomRequest.AddFeatureRequest(MakeShared<FBucketRoomFeatureRequest>(BucketId));
    }

    // Turn on invites.
    CreateRoomRequest.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(false));

    // Turn on join-by-ID.
    CreateRoomRequest.AddFeatureRequest(MakeShared<FJoinByIdRoomFeatureRequest>());

    // Turn on public member lists; this allows us to see the members in the lobby from search results alone in EOS
    // SDK 1.16.0 and later.
    CreateRoomRequest.AddFeatureRequest(MakeShared<FPublicMemberListRoomFeatureRequest>());

    // Turn on voice chat if requested.
    bool bVoiceChatEnabled = false;
    bool bEnableEcho = false;
    {
        if (Transaction.SetMetadata.Contains("EOSVoiceChat_Enabled") &&
            Transaction.SetMetadata["EOSVoiceChat_Enabled"].GetType() == EOnlineKeyValuePairDataType::Bool)
        {
            Transaction.SetMetadata["EOSVoiceChat_Enabled"].GetValue(bVoiceChatEnabled);
        }
#if !defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING
        if (Transaction.SetMetadata.Contains("EOSVoiceChat_Echo") &&
            Transaction.SetMetadata["EOSVoiceChat_Echo"].GetType() == EOnlineKeyValuePairDataType::Bool)
        {
            Transaction.SetMetadata["EOSVoiceChat_Echo"].GetValue(bEnableEcho);
        }
#endif
        if (bVoiceChatEnabled)
        {
            CreateRoomRequest.AddFeatureRequest(MakeShared<FVoiceChatRoomFeatureRequest>(
                bEnableEcho ? RTC::EJoinRoomOptions::EnableEcho : RTC::EJoinRoomOptions::None,
                false,
                false,
                false));
        }
    }

    // Add or update attributes as requested.
    for (const auto &Metadata : Transaction.SetMetadata)
    {
        // Handle the special bucket ID attribute.
        if (Metadata.Key == EOS_LOBBY_SEARCH_BUCKET_ID)
        {
            CreateRoomRequest.AddFeatureRequest(MakeShared<FBucketRoomFeatureRequest>(Metadata.Value.ToString()));
            continue;
        }

        switch (Metadata.Value.GetType())
        {
        case EOnlineKeyValuePairDataType::Bool: {
            bool AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            CreateRoomRequest.Attributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::Int64: {
            int64 AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            CreateRoomRequest.Attributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::Double: {
            double AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            CreateRoomRequest.Attributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::String: {
            FString AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            CreateRoomRequest.Attributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        default: {
            OnComplete.ExecuteIfBound(
                AsOSSError(Redpoint::EOS::API::ConvertError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(TEXT("The attribute '%s' has an unsupported type of value"), *Metadata.Key),
                    EOS_EResult::EOS_InvalidParameters)),
                UserId,
                nullptr);
            return true;
        }
        }
    }

    // Execute the create room request.
    this->RoomSystem->CreateRoom(
        CreateRoomRequest,
        IRoomSystem::FOnCreateRoomComplete::CreateSPLambda(
            this,
            [this, UserId = StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared()), OnComplete, bEnableEcho](
                FError ErrorCode,
                FRoomPtr Room) {
                if (!ErrorCode.WasSuccessful())
                {
                    OnComplete.ExecuteIfBound(AsOSSError(ErrorCode), *UserId, nullptr);
                    return;
                }

                auto LobbyId = FOnlinePartyIdRedpointBase::Create(Room->GetRoomId());
                if (bEnableEcho)
                {
                    this->VoiceManager->MarkPartyOrLobbyAsEchoEnabled(UserId, LobbyId);
                }
                TSharedRef<FOnlineLobbyEOS> ResultLobby = MakeShared<FOnlineLobbyEOS>(Room.ToSharedRef());
                this->RegisterEventsForNewRoom(Room.ToSharedRef());
                this->VoiceManager->UserManuallyJoinedLobby(*UserId, *LobbyId);
                OnComplete.ExecuteIfBound(AsOSSError(ErrorCode), *UserId, ResultLobby);
            }));
    return true;
}

bool FOnlineLobbyInterfaceEOS::UpdateLobby(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FOnlineLobbyTransaction &Transaction,
    FOnLobbyOperationComplete OnComplete)
{
    // Verify we have an EOS user as the actor.
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        OnComplete.ExecuteIfBound(AsOSSError(Errors::InvalidUser()), UserId);
        return true;
    }

    // Get the current room for the user so we can update it.
    FRoomPtr RoomPtr = this->GetCurrentRoomByPredicate(GetAccountId(UserId), [&LobbyId](const FRoomRef &Room) {
        return Room->GetRoomId()->GetRoomId() == LobbyId.ToString();
    });
    if (!RoomPtr.IsValid())
    {
        OnComplete.ExecuteIfBound(
            AsOSSError(Errors::InvalidRequest(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The lobby ID passed to UpdateLobby is not known to the room system."))),
            UserId);
        return true;
    }
    FRoomRef Room = RoomPtr.ToSharedRef();

    // Create the request.
    FUpdateRoomRequest UpdateRoomRequest = FUpdateRoomRequest();

    // Update the capacity of the lobby if requested.
    if (Transaction.Capacity.IsSet())
    {
        UpdateRoomRequest.MaxMemberCount = Transaction.Capacity.GetValue();
    }

    // Update the permissions of the lobby if requested.
    if (Transaction.Public.IsSet())
    {
        if (Transaction.Public.GetValue())
        {
            UpdateRoomRequest.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
        }
        else
        {
            UpdateRoomRequest.DeleteFeature(FPublicAdvertisementRoomFeatureRequest::GetFeatureNameStatic());
        }
    }

    // Add or update attributes as requested.
    for (const auto &Metadata : Transaction.SetMetadata)
    {
        // Handle the special bucket ID attribute.
        if (Metadata.Key == EOS_LOBBY_SEARCH_BUCKET_ID)
        {
            UpdateRoomRequest.AddFeatureRequest(MakeShared<FBucketRoomFeatureRequest>(Metadata.Value.ToString()));
            continue;
        }

        switch (Metadata.Value.GetType())
        {
        case EOnlineKeyValuePairDataType::Bool: {
            bool AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            UpdateRoomRequest.UpsertAttributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::Int64: {
            int64 AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            UpdateRoomRequest.UpsertAttributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::Double: {
            double AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            UpdateRoomRequest.UpsertAttributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::String: {
            FString AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            UpdateRoomRequest.UpsertAttributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        default: {
            OnComplete.ExecuteIfBound(
                AsOSSError(Redpoint::EOS::API::ConvertError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(TEXT("The attribute '%s' has an unsupported type of value"), *Metadata.Key),
                    EOS_EResult::EOS_InvalidParameters)),
                UserId);
            return true;
        }
        }
    }

    // Remove attributes as requested.
    for (const auto &MetadataKey : Transaction.DeleteMetadata)
    {
        UpdateRoomRequest.DeleteAttributes.Add(MetadataKey);
    }

    // Execute the update room request.
    this->RoomSystem->UpdateRoom(
        Room,
        UpdateRoomRequest,
        IRoomSystem::FOnUpdateRoomComplete::CreateSPLambda(
            this,
            [this, UserId = StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared()), OnComplete](
                FError ErrorCode) {
                OnComplete.ExecuteIfBound(AsOSSError(ErrorCode), *UserId);
            }));
    return true;
}

bool FOnlineLobbyInterfaceEOS::DeleteLobby(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    FOnLobbyOperationComplete OnComplete)
{
    // Verify we have an EOS user as the actor.
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        OnComplete.ExecuteIfBound(AsOSSError(Errors::InvalidUser()), UserId);
        return true;
    }

    // Get the current room for the user so we can update it.
    FRoomPtr RoomPtr = this->GetCurrentRoomByPredicate(GetAccountId(UserId), [&LobbyId](const FRoomRef &Room) {
        return Room->GetRoomId()->GetRoomId() == LobbyId.ToString();
    });
    if (!RoomPtr.IsValid())
    {
        // If the room system doesn't know about the room, treat it as a success (since the user is no longer in the
        // room they want to delete or disconnect from).
        OnComplete.ExecuteIfBound(AsOSSError(Errors::Success()), UserId);
        return true;
    }
    FRoomRef Room = RoomPtr.ToSharedRef();

    // Create and execute the leave room request.
    FLeaveRoomRequest LeaveRoomRequest = FLeaveRoomRequest();
    this->RoomSystem->LeaveRoom(
        Room,
        LeaveRoomRequest,
        IRoomSystem::FOnLeaveRoomComplete::CreateSPLambda(
            this,
            [this,
             Room,
             UserId = StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared()),
             LobbyId = StaticCastSharedRef<const FOnlinePartyIdRedpointBase>(LobbyId.AsShared()),
             OnComplete](FError ErrorCode) {
                // We call OnRoomLeft manually here to ensure that it fires before we pass control back to the
                // callback, to ensure the new code is consistent with the old behaviour. The room system will raise
                // an OnRoomLeft event afterwards due to synchronisation, but we'll ignore it because our OnRoomLeft
                // handler will already have removed the room's member event handlers.
                this->OnRoomLeft(
                    FRoomEventContext::Create(GetAccountId(UserId)),
                    Room,
                    ERoomLeaveReason::LeftExplicitly);

                // @todo: Is this still necessary? Probably for now as the voice manager will still be using
                // non-room-system APIs.
                this->VoiceManager->UserManuallyLeftLobby(*UserId, *LobbyId);

                OnComplete.ExecuteIfBound(AsOSSError(ErrorCode), *UserId);
            }));
    return true;
}

bool FOnlineLobbyInterfaceEOS::ConnectLobby(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    FOnLobbyCreateOrConnectComplete OnComplete)
{
    // Verify we have an EOS user as the actor.
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        OnComplete.ExecuteIfBound(AsOSSError(Errors::InvalidUser()), UserId, nullptr);
        return true;
    }

    // Join the lobby directly by ID. Since we now enable join-by-ID, we no longer need to do a search here.
    FJoinRoomRequest JoinRoomRequest = FJoinRoomRequest(
        ERoomScope::User,
        GetAccountId(UserId),
        MakeShared<FRoomId>(FName(TEXT("Lobby")), LobbyId.ToString()));
    this->RoomSystem->JoinRoom(
        JoinRoomRequest,
        IRoomSystem::FOnJoinRoomComplete::CreateSPLambda(
            this,
            [this,
             UserId = StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared()),
             OnComplete](FError ErrorCode, FRoomPtr Room) {
                if (!ErrorCode.WasSuccessful())
                {
                    OnComplete.ExecuteIfBound(AsOSSError(ErrorCode), *UserId, nullptr);
                    return;
                }

                auto LobbyId = FOnlinePartyIdRedpointBase::Create(Room->GetRoomId());
                TSharedRef<FOnlineLobbyEOS> ResultLobby = MakeShared<FOnlineLobbyEOS>(Room.ToSharedRef());
                this->RegisterEventsForNewRoom(Room.ToSharedRef());
                this->VoiceManager->UserManuallyJoinedLobby(*UserId, *LobbyId);
                OnComplete.ExecuteIfBound(AsOSSError(ErrorCode), *UserId, ResultLobby);
            }));
    return true;
}

bool FOnlineLobbyInterfaceEOS::DisconnectLobby(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    FOnLobbyOperationComplete OnComplete)
{
    // @note: In EOS, this is the same operation because we have owner migration turned off.
    return this->DeleteLobby(UserId, LobbyId, OnComplete);
}

bool FOnlineLobbyInterfaceEOS::UpdateMemberSelf(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FOnlineLobbyMemberTransaction &Transaction,
    FOnLobbyOperationComplete OnComplete)
{
    // Verify we have an EOS user as the actor.
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        OnComplete.ExecuteIfBound(AsOSSError(Errors::InvalidUser()), UserId);
        return true;
    }

    // Get the current room for the user so we can update it.
    FRoomPtr RoomPtr = this->GetCurrentRoomByPredicate(GetAccountId(UserId), [&LobbyId](const FRoomRef &Room) {
        return Room->GetRoomId()->GetRoomId() == LobbyId.ToString();
    });
    if (!RoomPtr.IsValid())
    {
        OnComplete.ExecuteIfBound(
            AsOSSError(Errors::InvalidRequest(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The lobby ID passed to UpdateMemberSelf is not known to the room system."))),
            UserId);
        return true;
    }
    FRoomRef Room = RoomPtr.ToSharedRef();

    // Create the request.
    FUpdateRoomRequest UpdateRoomRequest = FUpdateRoomRequest();

    // Add or update attributes as requested.
    for (const auto &Metadata : Transaction.SetMetadata)
    {
        // Handle the special bucket ID attribute.
        if (Metadata.Key == EOS_LOBBY_SEARCH_BUCKET_ID)
        {
            UpdateRoomRequest.AddFeatureRequest(MakeShared<FBucketRoomFeatureRequest>(Metadata.Value.ToString()));
            continue;
        }

        switch (Metadata.Value.GetType())
        {
        case EOnlineKeyValuePairDataType::Bool: {
            bool AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            UpdateRoomRequest.UpsertMemberAttributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::Int64: {
            int64 AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            UpdateRoomRequest.UpsertMemberAttributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::Double: {
            double AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            UpdateRoomRequest.UpsertMemberAttributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::String: {
            FString AttributeValue;
            Metadata.Value.GetValue(AttributeValue);
            UpdateRoomRequest.UpsertMemberAttributes.Add(Metadata.Key, AttributeValue);
            break;
        }
        default: {
            OnComplete.ExecuteIfBound(
                AsOSSError(Redpoint::EOS::API::ConvertError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(TEXT("The attribute '%s' has an unsupported type of value"), *Metadata.Key),
                    EOS_EResult::EOS_InvalidParameters)),
                UserId);
            return true;
        }
        }
    }

    // Remove attributes as requested.
    for (const auto &MetadataKey : Transaction.DeleteMetadata)
    {
        UpdateRoomRequest.DeleteMemberAttributes.Add(MetadataKey);
    }

    // Execute the update room request.
    this->RoomSystem->UpdateRoom(
        Room,
        UpdateRoomRequest,
        IRoomSystem::FOnUpdateRoomComplete::CreateSPLambda(
            this,
            [this, UserId = StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared()), OnComplete](
                FError ErrorCode) {
                OnComplete.ExecuteIfBound(AsOSSError(ErrorCode), *UserId);
            }));
    return true;
}

bool FOnlineLobbyInterfaceEOS::GetMemberCount(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    int32 &OutMemberCount)
{
    // Verify we have an EOS user as the actor.
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        OutMemberCount = 0;
        return false;
    }

    // If we're in the target room, use the live data.
    FRoomPtr RoomPtr = this->GetCurrentRoomByPredicate(GetAccountId(UserId), [&LobbyId](const FRoomRef &Room) {
        return Room->GetRoomId()->GetRoomId() == LobbyId.ToString();
    });
    if (RoomPtr.IsValid())
    {
        OutMemberCount = RoomPtr->GetMembers().Num();
        return true;
    }

    // Otherwise see if we have a recent search result to pull data from.
    FRoomSearchResultPtr SearchPtr =
        this->LastSearchResultsByUser.FindOrAdd(UserId.AsShared()).FindRef(LobbyId.ToString());
    if (SearchPtr.IsValid())
    {
        auto PublicMemberList = SearchPtr->GetRoomFeatureSearchState<IPublicMemberListRoomFeatureSearchState>();
        if (PublicMemberList.IsValid())
        {
            OutMemberCount = PublicMemberList->CurrentMembers().Num();
            return true;
        }
    }

    // We can't get the data from anywhere.
    OutMemberCount = 0;
    return false;
}

bool FOnlineLobbyInterfaceEOS::GetMemberUserId(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    int32 MemberIndex,
    TSharedPtr<const FUniqueNetId> &OutMemberId)
{
    // Verify we have an EOS user as the actor.
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        OutMemberId = nullptr;
        return false;
    }

    // If we're in the target room, use the live data.
    FRoomPtr RoomPtr = this->GetCurrentRoomByPredicate(GetAccountId(UserId), [&LobbyId](const FRoomRef &Room) {
        return Room->GetRoomId()->GetRoomId() == LobbyId.ToString();
    });
    if (RoomPtr.IsValid())
    {
        const auto &Members = RoomPtr->GetMembers();
        if (MemberIndex < 0 || MemberIndex >= Members.Num())
        {
            OutMemberId = nullptr;
            return false;
        }
        OutMemberId = GetUniqueNetId(Members[MemberIndex]->GetUserId());
        return true;
    }

    // Otherwise see if we have a recent search result to pull data from.
    FRoomSearchResultPtr SearchPtr =
        this->LastSearchResultsByUser.FindOrAdd(UserId.AsShared()).FindRef(LobbyId.ToString());
    if (SearchPtr.IsValid())
    {
        auto PublicMemberList = SearchPtr->GetRoomFeatureSearchState<IPublicMemberListRoomFeatureSearchState>();
        if (PublicMemberList.IsValid())
        {
            const auto &Members = PublicMemberList->CurrentMembers();
            if (MemberIndex < 0 || MemberIndex >= Members.Num())
            {
                OutMemberId = nullptr;
                return false;
            }
            OutMemberId = GetUniqueNetId(Members[MemberIndex]);
            return true;
        }
    }

    // We can't get the data from anywhere.
    OutMemberId = nullptr;
    return false;
}

bool FOnlineLobbyInterfaceEOS::GetMemberMetadataValue(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId,
    const FString &MetadataKey,
    FVariantData &OutMetadataValue)
{
    // Verify we have an EOS user as the actor.
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM || MemberId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        OutMetadataValue = FVariantData();
        return false;
    }

    // Member attributes can only be fetched if we're in the room.
    FRoomPtr RoomPtr = this->GetCurrentRoomByPredicate(GetAccountId(UserId), [&LobbyId](const FRoomRef &Room) {
        return Room->GetRoomId()->GetRoomId() == LobbyId.ToString();
    });
    if (!RoomPtr.IsValid())
    {
        OutMetadataValue = FVariantData();
        return false;
    }
    FRoomRef Room = RoomPtr.ToSharedRef();

    // Get the member, if they're present.
    auto MemberAccountId = GetAccountId(MemberId);
    const FRoomMemberRef *MemberPtr =
        Room->GetMembers().FindByPredicate([MemberAccountId](const FRoomMemberRef &Member) {
            return Member->GetUserId() == MemberAccountId;
        });
    if (MemberPtr == nullptr)
    {
        OutMetadataValue = FVariantData();
        return false;
    }
    const FRoomMemberRef &Member = *MemberPtr;

    // Get the attribute, if it is set.
    const FRoomAttribute *AttributePtr = Member->GetAttributes().Find(MetadataKey);
    if (AttributePtr == nullptr)
    {
        OutMetadataValue = FVariantData();
        return false;
    }
    const FRoomAttribute &Attribute = *AttributePtr;
    switch (Attribute.Type())
    {
    case FRoomAttribute::EType::Bool:
        OutMetadataValue = Attribute.GetBool();
        return true;
    case FRoomAttribute::EType::Int64:
        OutMetadataValue = Attribute.GetInt64();
        return true;
    case FRoomAttribute::EType::Double:
        OutMetadataValue = Attribute.GetDouble();
        return true;
    case FRoomAttribute::EType::String:
        OutMetadataValue = Attribute.GetString();
        return true;
    default:
        OutMetadataValue = FVariantData();
        return false;
    }
}

bool FOnlineLobbyInterfaceEOS::Search(
    const FUniqueNetId &UserId,
    const FOnlineLobbySearchQuery &Query,
    FOnLobbySearchComplete OnComplete)
{
    // Verify we have an EOS user as the actor.
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        OnComplete.ExecuteIfBound(
            AsOSSError(Errors::InvalidUser()),
            UserId,
            TArray<TSharedRef<const FOnlineLobbyId>>());
        return true;
    }

    // Convert the query into attribute constraints.
    TMap<FString, FRoomAttributeConstraint> AttributeConstraints;
    for (const auto &Filter : Query.Filters)
    {
        // Convert the filter value type.
        FRoomAttribute Attribute;
        switch (Filter.Value.GetType())
        {
        case EOnlineKeyValuePairDataType::Bool: {
            bool AttributeValue;
            Filter.Value.GetValue(AttributeValue);
            Attribute = FRoomAttribute(AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::Int64: {
            int64 AttributeValue;
            Filter.Value.GetValue(AttributeValue);
            Attribute = FRoomAttribute(AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::Double: {
            double AttributeValue;
            Filter.Value.GetValue(AttributeValue);
            Attribute = FRoomAttribute(AttributeValue);
            break;
        }
        case EOnlineKeyValuePairDataType::String: {
            FString AttributeValue;
            Filter.Value.GetValue(AttributeValue);
            Attribute = FRoomAttribute(AttributeValue);
            break;
        }
        default: {
            OnComplete.ExecuteIfBound(
                AsOSSError(Redpoint::EOS::API::ConvertError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(TEXT("The filter '%s' has an unsupported type of value"), *Filter.Key),
                    EOS_EResult::EOS_InvalidParameters)),
                UserId,
                TArray<TSharedRef<const FOnlineLobbyId>>());
            return true;
        }
        }

        // Convert the filter comparator.
        FRoomAttributeConstraint::EConstraintType ConstraintType = FRoomAttributeConstraint::EConstraintType::Equal;
        switch (Filter.Comparison)
        {
        case EOnlineLobbySearchQueryFilterComparator::Equal: {
            ConstraintType = FRoomAttributeConstraint::EConstraintType::Equal;
            break;
        }
        case EOnlineLobbySearchQueryFilterComparator::NotEqual: {
            ConstraintType = FRoomAttributeConstraint::EConstraintType::NotEqual;
            break;
        }
        case EOnlineLobbySearchQueryFilterComparator::LessThan: {
            ConstraintType = FRoomAttributeConstraint::EConstraintType::LessThan;
            break;
        }
        case EOnlineLobbySearchQueryFilterComparator::LessThanOrEqual: {
            ConstraintType = FRoomAttributeConstraint::EConstraintType::LessThanOrEqual;
            break;
        }
        case EOnlineLobbySearchQueryFilterComparator::GreaterThan: {
            ConstraintType = FRoomAttributeConstraint::EConstraintType::GreaterThan;
            break;
        }
        case EOnlineLobbySearchQueryFilterComparator::GreaterThanOrEqual: {
            ConstraintType = FRoomAttributeConstraint::EConstraintType::GreaterThanOrEqual;
            break;
        }
        case EOnlineLobbySearchQueryFilterComparator::Distance: {
            ConstraintType = FRoomAttributeConstraint::EConstraintType::Distance;
            break;
        }
        default: {
            OnComplete.ExecuteIfBound(
                AsOSSError(Redpoint::EOS::API::ConvertError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(TEXT("The filter '%s' has an unsupported comparator type"), *Filter.Key),
                    EOS_EResult::EOS_InvalidParameters)),
                UserId,
                TArray<TSharedRef<const FOnlineLobbyId>>());
            return true;
        }
        }

        // Add the constraint.
        AttributeConstraints.Add(Filter.Key, FRoomAttributeConstraint(Attribute, ConstraintType));
    }

    // Create the search request and execute it.
    FSearchRoomsRequest SearchRoomsRequest =
        FSearchRoomsRequest(GetAccountId(UserId), FName(TEXT("Lobby")), NAME_Game, AttributeConstraints);
    SearchRoomsRequest.MaxResultCount = Query.Limit;
    this->RoomSystem->SearchRooms(
        SearchRoomsRequest,
        IRoomSystem::FOnSearchRoomsComplete::CreateSPLambda(
            this,
            [this,
             UserId = StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared()),
             OnComplete](FError ErrorCode, TArray<FRoomSearchResultRef> RoomSearchResults) {
                if (!ErrorCode.WasSuccessful())
                {
                    OnComplete.ExecuteIfBound(
                        AsOSSError(ErrorCode),
                        *UserId,
                        TArray<TSharedRef<const FOnlineLobbyId>>());
                    return;
                }

                TMap<FString, FRoomSearchResultPtr> ResultsByLobbyId;
                TArray<TSharedRef<const FOnlineLobbyId>> ResultLobbyIds;
                for (const auto &RoomSearchResult : RoomSearchResults)
                {
                    ResultsByLobbyId.Add(RoomSearchResult->GetRoomId()->GetRoomId(), RoomSearchResult);
                    ResultLobbyIds.Add(FOnlinePartyIdRedpointBase::Create(RoomSearchResult->GetRoomId()));
                }
                this->LastSearchResultsByUser.Add(*UserId, ResultsByLobbyId);
                OnComplete.ExecuteIfBound(AsOSSError(ErrorCode), *UserId, ResultLobbyIds);
            }));
    return true;
}

bool FOnlineLobbyInterfaceEOS::GetLobbyMetadataValue(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FString &MetadataKey,
    FVariantData &OutMetadataValue)
{
    // Verify we have an EOS user as the actor.
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        OutMetadataValue = FVariantData();
        return false;
    }

    // Store the pointer to our potential attribute, so we can unify the presence check and value conversion.
    const FRoomAttribute *AttributePtr = nullptr;

    // If we're in the target room, use the live data.
    FRoomPtr RoomPtr = this->GetCurrentRoomByPredicate(GetAccountId(UserId), [&LobbyId](const FRoomRef &Room) {
        return Room->GetRoomId()->GetRoomId() == LobbyId.ToString();
    });
    if (RoomPtr.IsValid())
    {
        AttributePtr = RoomPtr->GetRoomAttributes().Find(MetadataKey);
    }
    else
    {
        // Otherwise see if we have a recent search result to pull data from.
        FRoomSearchResultPtr SearchPtr =
            this->LastSearchResultsByUser.FindOrAdd(UserId.AsShared()).FindRef(LobbyId.ToString());
        if (SearchPtr.IsValid())
        {
            AttributePtr = SearchPtr->GetRoomAttributes().Find(MetadataKey);
        }
    }

    // Ensure the attribute was found.
    if (AttributePtr == nullptr)
    {
        OutMetadataValue = FVariantData();
        return false;
    }

    // Otherwise return the attribute data.
    const FRoomAttribute &Attribute = *AttributePtr;
    switch (Attribute.Type())
    {
    case FRoomAttribute::EType::Bool:
        OutMetadataValue = Attribute.GetBool();
        return true;
    case FRoomAttribute::EType::Int64:
        OutMetadataValue = Attribute.GetInt64();
        return true;
    case FRoomAttribute::EType::Double:
        OutMetadataValue = Attribute.GetDouble();
        return true;
    case FRoomAttribute::EType::String:
        OutMetadataValue = Attribute.GetString();
        return true;
    default:
        OutMetadataValue = FVariantData();
        return false;
    }
}

TSharedPtr<FOnlineLobbyId> FOnlineLobbyInterfaceEOS::ParseSerializedLobbyId(const FString &InLobbyId)
{
    if (InLobbyId.StartsWith(TEXT("Lobby:")))
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Warning,
            TEXT("Detected string '%s' starts with 'Lobby:' when calling ParseSerializedLobbyId. This likely indicates "
                 "you're calling 'ToDebugString' on a party or lobby ID, when you should be calling 'ToString'. We'll "
                 "automatically fix this up for you, but you should trace where this value is coming from and call the "
                 "right string function."),
            *InLobbyId);
    }

    return ConstCastSharedRef<FOnlinePartyIdRedpointBase>(FOnlinePartyIdRedpointBase::Create(MakeShared<FRoomId>(
        FName(TEXT("Lobby")),
        InLobbyId.StartsWith(TEXT("Lobby:")) ? InLobbyId.Mid(6) : InLobbyId)));
}

bool FOnlineLobbyInterfaceEOS::KickMember(
    const FUniqueNetId &UserId,
    const FOnlineLobbyId &LobbyId,
    const FUniqueNetId &MemberId,
    FOnLobbyOperationComplete OnComplete)
{
    // Verify we have an EOS user as the actor.
    if (UserId.GetType() != REDPOINT_EOS_SUBSYSTEM || MemberId.GetType() != REDPOINT_EOS_SUBSYSTEM)
    {
        OnComplete.ExecuteIfBound(AsOSSError(Errors::InvalidUser()), UserId);
        return true;
    }

    // Get the current room for the user so we can kick the member from it.
    FRoomPtr RoomPtr = this->GetCurrentRoomByPredicate(GetAccountId(UserId), [&LobbyId](const FRoomRef &Room) {
        return Room->GetRoomId()->GetRoomId() == LobbyId.ToString();
    });
    if (!RoomPtr.IsValid())
    {
        OnComplete.ExecuteIfBound(
            AsOSSError(Errors::InvalidRequest(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The lobby ID passed to KickMember is not known to the room system."))),
            UserId);
        return true;
    }
    FRoomRef Room = RoomPtr.ToSharedRef();

    // Create and execute the kick room member request.
    FKickRoomMemberRequest KickRoomMemberRequest = FKickRoomMemberRequest();
    KickRoomMemberRequest.TargetUser = GetAccountId(MemberId);
    this->RoomSystem->KickRoomMember(
        Room,
        KickRoomMemberRequest,
        IRoomSystem::FOnKickRoomMemberComplete::CreateSPLambda(
            this,
            [this,
             UserId = StaticCastSharedRef<const FUniqueNetIdEOS>(UserId.AsShared()),
             LobbyId = LobbyId.AsShared(),
             OnComplete](FError ErrorCode) {
                OnComplete.ExecuteIfBound(AsOSSError(ErrorCode), *UserId);
            }));
    return true;
}

}

#endif // #if !REDPOINT_EOS_IS_DEDICATED_SERVER