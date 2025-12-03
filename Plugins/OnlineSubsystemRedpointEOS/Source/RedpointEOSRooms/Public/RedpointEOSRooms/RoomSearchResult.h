// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"
#include "RedpointEOSRooms/RoomAttribute.h"
#include "RedpointEOSRooms/RoomId.h"
#include "RedpointEOSRooms/RoomOwner.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2575674398, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API IRoomSearchResult
{
public:
    IRoomSearchResult() = default;
    UE_NONCOPYABLE(IRoomSearchResult);
    virtual ~IRoomSearchResult() = default;

    /**
     * Returns the ID of the room.
     */
    virtual FRoomIdRef GetRoomId() const = 0;

    /**
     * Returns the namespace of the room.
     */
    virtual FName GetRoomNamespace() const = 0;

    /**
     * Returns the owner of the room.
     */
    virtual FRoomOwnerRef GetRoomOwner() const = 0;

    /**
     * Returns the maximum number of members in the room.
     */
    virtual uint32 GetRoomMaxMembers() const = 0;

    /**
     * Returns the room attributes.
     */
    virtual const TMap<FString, FRoomAttribute> &GetRoomAttributes() const = 0;

    /**
     * If this search result was obtained through an overlay event, this is the UI event ID. You should not normally
     * need to access this value, as the UI event will automatically be acknowledged when IRoomSystem::JoinRoom is
     * called with this search result.
     */
    virtual TOptional<EOS_UI_EventId> GetAssociatedUiEventId() const = 0;

    /**
     * Returns a list of feature names that this room search result has.
     */
    virtual TSet<FName> GetRoomFeatureSearchStateNames() const = 0;

    /**
     * Returns the base feature search state if the room has the feature.
     */
    virtual TSharedPtr<Features::IRoomFeatureSearchStateBase> GetRoomFeatureSearchStateBase(
        const FName &FeatureName) const = 0;

    /**
     * Returns whether the room has the specified feature.
     */
    template <typename TRoomFeatureSearchState> bool HasRoomFeatureSearchState() const
    {
        static_assert(
            std::is_convertible_v<TRoomFeatureSearchState *, Features::IRoomFeatureSearchStateBase *>,
            "TRoomFeatureSearchState must inherit from IRoomFeatureSearchState.");
        return this->GetRoomFeatureSearchStateBase(TRoomFeatureSearchState::GetFeatureNameStatic()).IsValid();
    }

    /**
     * Returns the feature search state if the room has the feature.
     */
    template <typename TRoomFeatureSearchState>
    TSharedPtr<const TRoomFeatureSearchState> GetRoomFeatureSearchState() const
    {
        static_assert(
            std::is_convertible_v<TRoomFeatureSearchState *, Features::IRoomFeatureSearchStateBase *>,
            "TRoomFeatureSearchState must inherit from IRoomFeatureSearchState.");
        return StaticCastSharedPtr<TRoomFeatureSearchState>(
            this->GetRoomFeatureSearchStateBase(TRoomFeatureSearchState::GetFeatureNameStatic()));
    }

    /**
     * Returns a feature search state that the room has, or asserts if the room doesn't have the feature.
     */
    template <typename TRoomFeatureSearchState>
    TSharedRef<const TRoomFeatureSearchState> GetRoomFeatureSearchStateChecked() const
    {
        static_assert(
            std::is_convertible_v<TRoomFeatureSearchState *, Features::IRoomFeatureSearchStateBase *>,
            "TRoomFeatureSearchState must inherit from IRoomFeatureSearchState.");
        return this->GetRoomFeatureSearchState<TRoomFeatureSearchState>().ToSharedRef();
    }
};

typedef TSharedRef<IRoomSearchResult> FRoomSearchResultRef;
typedef TSharedPtr<IRoomSearchResult> FRoomSearchResultPtr;

}

namespace Redpoint::EOS::Rooms
{
REDPOINT_EOS_FILE_NS_EXPORT(2575674398, Redpoint::EOS::Rooms, IRoomSearchResult)
REDPOINT_EOS_FILE_NS_EXPORT(2575674398, Redpoint::EOS::Rooms, FRoomSearchResultRef)
REDPOINT_EOS_FILE_NS_EXPORT(2575674398, Redpoint::EOS::Rooms, FRoomSearchResultPtr)
}

REDPOINT_EOS_CODE_GUARD_END()