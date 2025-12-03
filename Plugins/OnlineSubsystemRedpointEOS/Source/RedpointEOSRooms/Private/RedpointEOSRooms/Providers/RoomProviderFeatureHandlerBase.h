// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error/Decl.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeature.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/RoomFeatureSearchState.h"
#include "RedpointEOSRooms/Providers/RoomProviderWriteOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3320738808, Redpoint::EOS::Rooms, IRoom)
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(3254663568, Redpoint::EOS::Rooms::Providers, FRoomProvider)

namespace REDPOINT_EOS_FILE_NS_ID(3343807417, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::API;

class FRoomProviderFeatureHandlerBase
{
public:
    struct FFeaturePostSynchronisationResult
    {
        enum class ERequireRoomUpdate
        {
            No,
            Yes,
        };

        enum class EStateUpdated
        {
            No,
            Yes,
        };

        FFeaturePostSynchronisationResult(ERequireRoomUpdate InRequireRoomUpdate, EStateUpdated InStateUpdated)
            : RequireRoomUpdate(InRequireRoomUpdate)
            , StateUpdated(InStateUpdated)
        {
        }

        /**
         * If true, the state of the feature has changed such that the underlying room on the EOS SDK must be updated
         * via an automatic UpdateRoom call. This allows features to react to changes such as members joining or
         * leaving, and update the attributes they sync to EOS based on those changes.
         */
        ERequireRoomUpdate RequireRoomUpdate;

        /**
         * If true, the publicly visible state of the room feature has changed and room synchronisation should consider
         * emit this feature as updated with EChangeType::Updated.
         */
        EStateUpdated StateUpdated;
    };

    enum class EExclusivityKeyBehaviour : uint8
    {
        /**
         * When attempting to join a room that conflicts with an existing room, reject creating or joining the new room
         * and fail the requested operation.
         */
        RejectNewRoom,

        /**
         * When attempting to join a room that conflicts with an existing room, leave the existing room prior to the new
         * room being joined so that there is no conflict.
         */
        ReplaceExistingRoom,
    };

    class FExclusivityKey
    {
    private:
        FString Key;
        EExclusivityKeyBehaviour Behaviour;

    public:
        FExclusivityKey(FString InKey, EExclusivityKeyBehaviour InBehaviour)
            : Key(InKey)
            , Behaviour(InBehaviour)
        {
        }

        FORCEINLINE const FString &GetKey() const
        {
            return this->Key;
        }

        FORCEINLINE const EExclusivityKeyBehaviour &GetBehaviour() const
        {
            return this->Behaviour;
        }
    };

    typedef TDelegate<TMap<FString, FRoomAttribute>(
        const FRoomProviderWriteOperationRef &WriteOp,
        const TSharedRef<IRoomFeatureRequestBase> &FeatureRequest)>
        FSerializeFeatureRequestToRoomAttributesBase;

    typedef TDelegate<TSharedPtr<IRoomFeature>(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedRef<IRoom> &Room)>
        FConstructFeatureImplementationBase;
    typedef TDelegate<void(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedRef<IRoom> &Room,
        const TSharedRef<IRoomFeature> &Feature)>
        FUpdateFeatureImplementationDuringPreSynchronisationBase;
    typedef TDelegate<FFeaturePostSynchronisationResult(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedRef<IRoom> &Room,
        const TSharedRef<IRoomFeature> &Feature,
        const TMap<FString, FRoomAttribute> &FeatureAttributes,
        const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes)>
        FUpdateFeatureImplementationDuringPostSynchronisationBase;
    typedef TDelegate<bool(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedRef<IRoom> &Room)>
        FTestFeaturePresenceFromLocalRoomState;

    typedef TDelegate<TSharedPtr<IRoomFeatureSearchStateBase>(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TMap<FString, FRoomAttribute> &FeatureAttributes)>
        FConstructFeatureSearchStateBase;
    typedef TDelegate<bool(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TMap<FString, FRoomAttribute> &FeatureAttributes)>
        FTestFeaturePresenceFromSearchReadOp;

    typedef TDelegate<FError(
        const FRoomProviderWriteOperationRef &WriteOp,
        const TSharedRef<IRoomFeatureRequestBase> &FeatureRequest)>
        FApplyFeatureRequestToWriteOperationBase;
    typedef TDelegate<FError(const FRoomProviderWriteOperationRef &WriteOp)>
        FApplyOmittedFeatureRequestToWriteOperationBase;
    typedef TDelegate<FError(
        const FRoomProviderWriteOperationRef &WriteOp,
        const TSharedPtr<IRoomFeatureRequestBase> &FeatureRequestOrNullIfDeleted,
        const TSharedPtr<IRoomFeature> &ExistingFeatureOrNullIfNotPresent)>
        FApplyFeatureRequestToUpdateOperationBase;

    typedef TDelegate<bool(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedRef<IRoom> &Room,
        const TSharedRef<IRoomFeature> &Feature,
        FRoomUserId RoomMemberId,
        const TMap<FString, FRoomAttribute> &FeatureMemberAttributes)>
        FShouldKickReadyRoomMemberAfterJoinBase;

    typedef TDelegate<FExclusivityKey(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedPtr<IRoom> &Room,
        const TSharedRef<IRoomFeatureRequestBase> &FeatureRequest)>
        FGetRoomExclusivityKeyFromFeatureRequestBase;
    typedef TDelegate<FExclusivityKey(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedRef<IRoom> &Room,
        const TSharedRef<IRoomFeature> &Feature)>
        FGetRoomExclusivityKeyFromFeatureBase;
    typedef TDelegate<FExclusivityKey(
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TSharedRef<IRoomFeatureSearchStateBase> &FeatureSearchState)>
        FGetRoomExclusivityKeyFromFeatureSearchStateBase;

public:
    FRoomProviderFeatureHandlerBase() = default;
    UE_NONCOPYABLE(FRoomProviderFeatureHandlerBase);
    virtual ~FRoomProviderFeatureHandlerBase() = default;

    virtual FName GetFeatureName() const = 0;

    virtual bool AllowOnCreate() const = 0;
    virtual bool AllowAddOnUpdate() const = 0;
    virtual bool AllowModifyOnUpdate() const = 0;
    virtual bool AllowDeleteOnUpdate() const = 0;
    virtual bool AllowOnJoin() const = 0;

    virtual FSerializeFeatureRequestToRoomAttributesBase SerializeFeatureRequestToRoomAttributes() const = 0;
    virtual FSerializeFeatureRequestToRoomAttributesBase SerializeFeatureRequestToRoomMemberAttributes() const = 0;

    virtual FConstructFeatureImplementationBase ConstructFeatureImplementation() const = 0;
    virtual FUpdateFeatureImplementationDuringPreSynchronisationBase
    UpdateFeatureImplementationDuringPreSynchronisation() const = 0;
    virtual FUpdateFeatureImplementationDuringPostSynchronisationBase
    UpdateFeatureImplementationDuringPostSynchronisation() const = 0;
    virtual FTestFeaturePresenceFromLocalRoomState TestFeaturePresenceFromLocalRoomState() const = 0;

    virtual FConstructFeatureSearchStateBase ConstructFeatureSearchState() const = 0;
    virtual FTestFeaturePresenceFromSearchReadOp TestFeaturePresenceFromSearchReadOp() const = 0;

    virtual FApplyFeatureRequestToWriteOperationBase ApplyFeatureRequestOnCreateRoomOperation() const = 0;
    virtual FApplyFeatureRequestToWriteOperationBase ApplyFeatureRequestOnPostCreateRoomOperation() const = 0;
    virtual FApplyOmittedFeatureRequestToWriteOperationBase ApplyOmittedFeatureRequestOnCreateRoomOperation() const = 0;
    virtual FApplyOmittedFeatureRequestToWriteOperationBase ApplyOmittedFeatureRequestOnPostCreateRoomOperation()
        const = 0;
    virtual FApplyFeatureRequestToUpdateOperationBase ApplyFeatureRequestOnUpdateRoomOperation() const = 0;
    virtual FApplyFeatureRequestToWriteOperationBase ApplyFeatureRequestOnJoinRoomOperation() const = 0;
    virtual FApplyFeatureRequestToWriteOperationBase ApplyFeatureRequestOnPostJoinRoomOperation() const = 0;

    virtual FShouldKickReadyRoomMemberAfterJoinBase ShouldKickReadyRoomMemberAfterJoin() const = 0;

    virtual FGetRoomExclusivityKeyFromFeatureRequestBase GetRoomExclusivityKeyFromFeatureRequest() const = 0;
    virtual FGetRoomExclusivityKeyFromFeatureBase GetRoomExclusivityKeyFromFeature() const = 0;
    virtual FGetRoomExclusivityKeyFromFeatureSearchStateBase GetRoomExclusivityKeyFromFeatureSearchState() const = 0;
};

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(3343807417, Redpoint::EOS::Rooms::Providers, FRoomProviderFeatureHandlerBase)
}

REDPOINT_EOS_CODE_GUARD_END()