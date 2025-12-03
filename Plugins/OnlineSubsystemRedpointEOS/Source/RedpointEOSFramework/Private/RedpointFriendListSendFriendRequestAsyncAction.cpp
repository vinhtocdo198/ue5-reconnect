// Copyright June Rhodes. All Rights Reserved.

#include "RedpointFriendListSendFriendRequestAsyncAction.h"

#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSFriends/FriendSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

URedpointFriendListSendFriendRequestAsyncAction *URedpointFriendListSendFriendRequestAsyncAction::SendFriendRequest(
    URedpointFriendListEntry *FriendListEntry)
{
    URedpointFriendListSendFriendRequestAsyncAction *Node =
        NewObject<URedpointFriendListSendFriendRequestAsyncAction>();
    Node->Entry = FriendListEntry;
    return Node;
}

void URedpointFriendListSendFriendRequestAsyncAction::Activate()
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    using namespace ::Redpoint::EOS::Core::Id;

    if (!IsValid(this->Entry) || !this->Entry->UserId.IsValid() || !IsValid(this->Entry->BoundForLocalPlayer))
    {
        this->OnComplete.Broadcast();
        return;
    }

    this->ActivateInternal();
#else
    this->OnComplete.Broadcast();
#endif
}

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

::Redpoint::EOS::Async::TTask<void> URedpointFriendListSendFriendRequestAsyncAction::ActivateInternal()
{
    using namespace ::Redpoint::EOS::Async;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Friends;

    auto PlatformHandle = FWorldResolution::GetPlatformHandle(this->Entry->BoundForLocalPlayer->GetWorld());
    auto FriendSystem = PlatformHandle->GetSystem<IFriendSystem>();

    this->Entry->bIsBeingActedUpon = true;

    IFriendSystem::FSendFriendRequestResult Result =
        co_await TTask<IFriendSystem::FSendFriendRequestResult>::FromFuture(
            FriendSystem->SendFriendRequest(Entry->BoundForLocalUserId, GetAccountId(this->Entry->UserId)));
    // @todo: Propagate errors in some standard way.

    this->Entry->bIsBeingActedUpon = false;
    this->OnComplete.Broadcast();

    co_return;
}

#endif

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()