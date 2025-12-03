// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"
#include "RedpointEOSRooms/Providers/RoomProviderReadOperation.h"
#include "RedpointEOSRooms/RoomAttribute.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSRooms/RoomOwner.h"
#include "RedpointEOSRooms/RoomSearchResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3254663568, Redpoint::EOS::Rooms::Providers, FRoomProvider)

namespace REDPOINT_EOS_FILE_NS_ID(1726319936, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Rooms::Features;

class FRoomSearchResultImpl : public IRoomSearchResult
{
    friend FRoomProvider;

protected:
    FRoomUserId BoundForLocalUser;
    FRoomIdRef RoomId;
    FName RoomNamespace;
    FRoomOwnerRef RoomOwner;
    uint32 RoomMaxMembers;
    TMap<FName, TSharedRef<IRoomFeatureSearchStateBase>> RoomFeatureSearchStates;
    TMap<FString, FRoomAttribute> RoomAttributes;
    FRoomProviderReadOperationRef ReadOperation;
    TOptional<EOS_UI_EventId> AssociatedUiEventId;

public:
    FRoomSearchResultImpl(
        const FRoomUserId &InBoundForLocalUser,
        const FRoomIdRef &InRoomId,
        const FName &InRoomNamespace,
        const FRoomOwnerRef &InRoomOwner,
        uint32 InRoomMaxMembers,
        const TMap<FName, TSharedRef<IRoomFeatureSearchStateBase>> &InRoomFeatureSearchStates,
        const TMap<FString, FRoomAttribute> &InRoomAttributes,
        const FRoomProviderReadOperationRef &InReadOperation,
        const TOptional<EOS_UI_EventId> &InAssociatedUiEventId);
    UE_NONCOPYABLE(FRoomSearchResultImpl);
    virtual ~FRoomSearchResultImpl() override = default;

    virtual FRoomIdRef GetRoomId() const override;
    virtual FName GetRoomNamespace() const override;
    virtual FRoomOwnerRef GetRoomOwner() const override;
    virtual uint32 GetRoomMaxMembers() const override;
    virtual const TMap<FString, FRoomAttribute> &GetRoomAttributes() const override;
    virtual TOptional<EOS_UI_EventId> GetAssociatedUiEventId() const override;
    virtual TSet<FName> GetRoomFeatureSearchStateNames() const override;
    virtual TSharedPtr<Features::IRoomFeatureSearchStateBase> GetRoomFeatureSearchStateBase(
        const FName &FeatureName) const override;

    FRoomUserId GetBoundForLocalUser() const;
    const FRoomProviderReadOperationRef &GetReadOperation() const;
};

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(1726319936, Redpoint::EOS::Rooms::Providers, FRoomSearchResultImpl)
}

REDPOINT_EOS_CODE_GUARD_END()