// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Requests/JoinRoomRequest.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/RoomRequestedJoinImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3330400371, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms::Requests;

FJoinRoomRequest::FJoinRoomRequest(
    ERoomScope InRoomScope,
    FRoomUserId InLocalUser,
    const FRoomSearchResultRef &InSearchResult)
    : RoomScope(InRoomScope)
    , LocalUser(InLocalUser)
    , SearchResult(InSearchResult)
    , RoomId(nullptr)
    , RoomInvite(nullptr)
    , RoomRequestedJoin(nullptr)
{
}

FJoinRoomRequest::FJoinRoomRequest(ERoomScope InRoomScope, FRoomUserId InLocalUser, const FRoomIdRef &InRoomId)
    : RoomScope(InRoomScope)
    , LocalUser(InLocalUser)
    , SearchResult(nullptr)
    , RoomId(InRoomId)
    , RoomInvite(nullptr)
    , RoomRequestedJoin(nullptr)
{
}

FJoinRoomRequest::FJoinRoomRequest(ERoomScope InRoomScope, const FRoomInviteRef &InRoomInvite)
    : RoomScope(InRoomScope)
    , LocalUser(InRoomInvite->GetRoomInviteRecipientUserId())
    , SearchResult(nullptr)
    , RoomId(nullptr)
    , RoomInvite(InRoomInvite)
    , RoomRequestedJoin(nullptr)
{
}

FJoinRoomRequest::FJoinRoomRequest(ERoomScope InRoomScope, const FRoomRequestedJoinRef &InRoomRequestedJoin)
    : RoomScope(InRoomScope)
    , LocalUser(InRoomRequestedJoin->GetActorUserId())
    , SearchResult(nullptr)
    , RoomId(nullptr)
    , RoomInvite(nullptr)
    , RoomRequestedJoin(InRoomRequestedJoin)
{
}

FRoomIdRef FJoinRoomRequest::GetRoomId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Requests);

    if (this->RoomId.IsValid())
    {
        return this->RoomId.ToSharedRef();
    }
    else if (this->SearchResult.IsValid())
    {
        return this->SearchResult->GetRoomId();
    }
    else if (this->RoomInvite.IsValid())
    {
        return this->RoomInvite->GetRoomInviteSearchResult()->GetRoomId();
    }
    else if (this->RoomRequestedJoin.IsValid())
    {
        return this->RoomRequestedJoin->GetRoomSearchResult()->GetRoomId();
    }
    checkf(false, TEXT("Invalid FJoinRoomRequest to call FJoinRoomRequest::GetRoomId on!"));
    return MakeShared<FRoomId>(NAME_None, TEXT(""));
}

FRoomInvitePtr FJoinRoomRequest::GetRoomInvite() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Requests);

    if (this->RoomInvite.IsValid())
    {
        return this->RoomInvite;
    }
    else if (this->RoomRequestedJoin.IsValid())
    {
        return StaticCastSharedRef<FRoomRequestedJoinImpl>(this->RoomRequestedJoin.ToSharedRef())->GetRoomInvite();
    }
    return nullptr;
}

}

REDPOINT_EOS_CODE_GUARD_END()