// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Policies/PrettyJsonPrintPolicy.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Logging.h"
#include "RedpointEOSRooms/Providers/RoomProviderActionOperation.h"
#include "RedpointEOSRooms/Providers/RoomProviderFeatureHandler.h"
#include "RedpointEOSRooms/Providers/RoomProviderFeatureHandlerBase.h"
#include "RedpointEOSRooms/Providers/RoomProviderFeatureSetHandler.h"
#include "RedpointEOSRooms/Providers/RoomProviderReadOperation.h"
#include "RedpointEOSRooms/Providers/RoomProviderSearchOperation.h"
#include "RedpointEOSRooms/Providers/RoomProviderWriteOperation.h"
#include "RedpointEOSRooms/RoomInviteId.h"
#include "RedpointEOSRooms/RoomSystem.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationContext.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(2020585069, Redpoint::EOS::Rooms::Database, FRoomDatabase)

namespace REDPOINT_EOS_FILE_NS_ID(3254663568, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Rooms::Database;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::API;

class FRoomProvider : public TSharedFromThis<FRoomProvider>
{
private:
    FPlatformHandle BasePlatformHandle;
    TMap<FName, TSharedPtr<FRoomProviderFeatureHandlerBase>> FeatureHandlers;
    TArray<TSharedRef<FRoomProviderFeatureSetHandler>> FeatureSetHandlers;
    TSet<FRoomRef> AutomaticRoomUpdatesInProgress;
    TSet<FRoomRef> DeferredAutomaticRoomUpdates;

    const FString ReservedRoomAttributePrefix = TEXT("Redpoint:EOS:");
    const FString FeatureNamesRoomAttribute = TEXT("Redpoint:EOS:FeatureNames");
    const FString ReadyRoomAttribute = TEXT("Redpoint:EOS:Ready");
    const FString ReadyRoomMemberAttribute = TEXT("Redpoint:EOS:Ready");
    const FString NamespaceFilterRoomAttribute = TEXT("Redpoint:EOS:NamespaceFilter");
    const FString NamespaceCasingRoomAttribute = TEXT("Redpoint:EOS:NamespaceCasing");
    const FString ForceUpdateIdentRoomAttribute = TEXT("Redpoint:EOS:ForceUpdateIdent");

private:
    template <typename TOperation> TSharedRef<TOperation> InitializeOperation(TSharedRef<TOperation> Operation)
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

#if !NO_LOGGING
        if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
        {
            Operation->InitializeDebugJson();
        }
#endif
        return Operation;
    }

#if !NO_LOGGING
    template <typename TOperation>
    TSharedRef<TOperation> DumpOperation(
        TSharedRef<TOperation> Operation,
        const FString &OperationName,
        const FString &Context) const
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
        {
            FString JsonString;
            auto JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonString);
            FJsonSerializer::Serialize(ConstCastSharedRef<FJsonObject>(Operation->GetDebugJson()), JsonWriter);
            JsonWriter->Close();

            UE_LOG(
                LogRedpointEOSRooms,
                VeryVerbose,
                TEXT("Executing '%s' with context '%s' and operation state: %s"),
                *OperationName,
                *Context,
                *JsonString);
        }
        return Operation;
    }
#endif

protected:
    template <
        typename TRoomFeature,
        typename TRoomFeatureImpl,
        typename TRoomFeatureRequest,
        typename TRoomFeatureSearchState>
    FRoomProviderFeatureHandler<TRoomFeature, TRoomFeatureImpl, TRoomFeatureRequest, TRoomFeatureSearchState> &
    RegisterFeatureHandler()
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        auto Handler = MakeShared<FRoomProviderFeatureHandler<
            TRoomFeature,
            TRoomFeatureImpl,
            TRoomFeatureRequest,
            TRoomFeatureSearchState>>();
        this->FeatureHandlers.Add(TRoomFeature::GetFeatureName(), Handler);
        return *Handler;
    }

    FRoomProviderFeatureSetHandler &RegisterFeatureSetHandler()
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        auto Handler = MakeShared<FRoomProviderFeatureSetHandler>();
        this->FeatureSetHandlers.Add(Handler);
        return *Handler;
    }

public:
    FRoomProvider(const FPlatformHandle &InBasePlatformHandle);
    UE_NONCOPYABLE(FRoomProvider);
    virtual ~FRoomProvider() = default;

    virtual FName GetProviderName() const = 0;

    const TMap<FName, TSharedPtr<FRoomProviderFeatureHandlerBase>> &GetFeatureHandlers() const;

protected:
    /**
     * Create an operation that will create a new room. Creation of rooms is split into two phases: "create"
     * and "post-create", since some room implementations don't support setting e.g. attributes as part of
     * the room creation and need it to be done in a post-create step.
     */
    virtual FRoomProviderWriteOperationRef CreateCreateRoomOperation(
        const FRoomUserId &UserId,
        const FName &RoomNamespace) = 0;

    /**
     * The callback type for the initial room creation. This callback expects to be handed an operation that
     * will be passed into ExecutePostCreateRoomOperation when the error code reports success.
     */
    typedef TDelegate<void(FError ErrorCode, FRoomProviderWriteOperationPtr PostCreateOperation)>
        FOnExecuteCreateRoomOperationComplete;

    /**
     * Called by the room system when a room needs to be created. The provided write operation will have
     * had all of the relevant APIs called on it (e.g. SetAttribute) to represent the desired state of
     * the room.
     */
    virtual void ExecuteCreateRoomOperation(
        const FRoomProviderWriteOperationRef &CreateOperation,
        const FOnExecuteCreateRoomOperationComplete &OnComplete) = 0;

    /**
     * The callback type for the post-create operation. This callback expects to be handed the created
     * room that will be provided to consumers of the room system APIs.
     */
    typedef TDelegate<void(FError ErrorCode, FRoomPtr Room)> FOnExecutePostCreateRoomOperationComplete;

    /**
     * Called by the room system after the room has been created, but before it is ready to be used
     * by consumers. Once this successfully completes, the room must match the state requested by
     * PostCreateOperation value.
     */
    virtual void ExecutePostCreateRoomOperation(
        const FRoomProviderWriteOperationRef &PostCreateOperation,
        const FOnExecutePostCreateRoomOperationComplete &OnComplete) = 0;

    /**
     * The callback type for cancelling a post-create operation.
     */
    typedef TDelegate<void(FError ErrorCode)> FOnCancelPostCreateRoomOperationComplete;

    /**
     * Called by the room system if the CreateRoom operation succeeds, but the PostCreateRoom operation fails,
     * to ensure that the room does not remain in a partially created state.
     */
    virtual void CancelPostCreateRoomOperation(
        const FRoomProviderWriteOperationRef &PostCreateOperation,
        const FOnCancelPostCreateRoomOperationComplete &OnComplete) = 0;

    /**
     * Create an operation that will update an existing room.
     */
    virtual FRoomProviderWriteOperationRef CreateUpdateRoomOperation(
        const FRoomUserId &UserId,
        const FRoomRef &Room) = 0;

    /**
     * The callback type for an update room operation.
     */
    typedef TDelegate<void(FError ErrorCode)> FOnExecuteUpdateRoomOperationComplete;

    /**
     * Called by the room system to update a room, after the UpdateOperation value has
     * had calls on it to represent the changes to the room. The implementation should
     * only act on the specific values requested on the UpdateOperation value, as the
     * room system will compute the differences between the current and desired states
     * (i.e. this implementation must not delete an attribute unless RemoveAttribute
     * was called on the UpdateOperation).
     */
    virtual void ExecuteUpdateRoomOperation(
        const FRoomProviderWriteOperationRef &UpdateOperation,
        const FOnExecuteUpdateRoomOperationComplete &OnComplete) = 0;

    /**
     * Creates an operation that will search for rooms.
     */
    virtual FRoomProviderSearchOperationRef CreateSearchRoomsOperation(
        const FRoomUserId &UserId,
        TOptional<uint32> MaxResults) = 0;

    /**
     * The callback type used when searching for rooms. The callback should return a read operation for each search
     * result, which the room provider can use to read information about the search result. It is expected that ReadOps
     * will only be populated if the error code is successful.
     */
    typedef TDelegate<void(FError ErrorCode, TArray<FRoomProviderReadOperationRef> ReadOps)>
        FOnExecuteSearchRoomsOperationComplete;

    /**
     * Called by the room system to search for rooms.
     */
    virtual void ExecuteSearchRoomsOperation(
        const FRoomProviderSearchOperationRef &SearchOperation,
        const FOnExecuteSearchRoomsOperationComplete &OnComplete) = 0;

    /**
     * Create an operation that will attempt to join a room by ID. Joining rooms is split into a "join"
     * phase and "post-join" phase, as some room implementations do not support setting member attributes
     * as part of the initial join operation.
     */
    virtual FRoomProviderWriteOperationRef CreateJoinRoomByIdOperation(
        const FRoomUserId &UserId,
        const FRoomIdRef &RoomId) = 0;

    /**
     * Create an operation that will attempt to join a room from a search result. The search result
     * will be a value previously returned from ExecuteSearchRoomsOperation. Joining rooms is split into
     * a "join" phase and "post-join" phase, as some room implementations do not support setting member
     * attributes as part of the initial join operation.
     */
    virtual FRoomProviderWriteOperationRef CreateJoinRoomBySearchResultOperation(
        const FRoomUserId &UserId,
        const FRoomSearchResultRef &RoomSearchResult) = 0;

    /**
     * The callback type for the initial room join. This callback expects to be handed an operation that
     * will be passed into ExecutePostJoinRoomOperation when the error code reports success.
     */
    typedef TDelegate<void(FError ErrorCode, FRoomProviderWriteOperationPtr PostJoinOperation)>
        FOnExecuteJoinRoomOperationComplete;

    /**
     * Called by the room system when a room needs to be joined. The provided write operation will have
     * had all of the relevant APIs called on it (e.g. SetMemberAttribute) to represent the desired state of
     * the room.
     */
    virtual void ExecuteJoinRoomOperation(
        const FRoomProviderWriteOperationRef &JoinOperation,
        const FOnExecuteJoinRoomOperationComplete &OnComplete) = 0;

    /**
     * The callback type for the post-join operation. This callback expects to be handed the joined
     * room that will be provided to consumers of the room system APIs.
     */
    typedef TDelegate<void(FError ErrorCode, FRoomPtr Room)> FOnExecutePostJoinRoomOperationComplete;

    /**
     * Called by the room system after the room has been joined, but before it is ready to be used
     * by consumers. Once this successfully completes, the room must match the state requested by
     * PostJoinOperation value.
     */
    virtual void ExecutePostJoinRoomOperation(
        const FRoomProviderWriteOperationRef &PostJoinOperation,
        const FOnExecutePostJoinRoomOperationComplete &OnComplete) = 0;

    /**
     * The callback type for cancelling a post-join operation.
     */
    typedef TDelegate<void(FError ErrorCode)> FOnCancelPostJoinRoomOperationComplete;

    /**
     * Called by the room system if the JoinRoom operation succeeds, but the PostJoinRoom operation fails,
     * to ensure that the room does not remain in a partially joined state.
     */
    virtual void CancelPostJoinRoomOperation(
        const FRoomProviderWriteOperationRef &PostJoinOperation,
        const FOnCancelPostJoinRoomOperationComplete &OnComplete) = 0;

    /**
     * Creates an operation that will leave a room that is currently joined.
     */
    virtual FRoomProviderActionOperationRef CreateLeaveRoomOperation(
        const FRoomUserId &UserId,
        const FRoomRef &Room) = 0;

    /**
     * The callback type for a leave room operation.
     */
    typedef TDelegate<void(FError ErrorCode)> FOnExecuteLeaveRoomOperationComplete;

    /**
     * Called by the room system to leave a room.
     */
    virtual void ExecuteLeaveRoomOperation(
        const FRoomProviderActionOperationRef &LeaveOperation,
        const FOnExecuteLeaveRoomOperationComplete &OnComplete) = 0;

    /**
     * Creates an operation that will promote another member to be the owner of a room.
     */
    virtual FRoomProviderActionOperationRef CreatePromoteRoomMemberOperation(
        const FRoomUserId &UserId,
        const FRoomRef &Room,
        const FRoomUserId &TargetUserId) = 0;

    /**
     * The callback type for a promote room member operation.
     */
    typedef TDelegate<void(FError ErrorCode)> FOnExecutePromoteRoomMemberOperationComplete;

    /**
     * Called by the room system to promote a room member to be the owner.
     */
    virtual void ExecutePromoteRoomMemberOperation(
        const FRoomProviderActionOperationRef &PromoteOperation,
        const FOnExecutePromoteRoomMemberOperationComplete &OnComplete) = 0;

    /**
     * Creates an operation that will kick another member from the room.
     */
    virtual FRoomProviderActionOperationRef CreateKickRoomMemberOperation(
        const FRoomUserId &UserId,
        const FRoomRef &Room,
        const FRoomUserId &TargetUserId) = 0;

    /**
     * The callback type for a kick room member operation.
     */
    typedef TDelegate<void(FError ErrorCode)> FOnExecuteKickRoomMemberOperationComplete;

    /**
     * Called by the room system to kick a room member from the room.
     */
    virtual void ExecuteKickRoomMemberOperation(
        const FRoomProviderActionOperationRef &KickOperation,
        const FOnExecuteKickRoomMemberOperationComplete &OnComplete) = 0;

    /**
     * Creates an operation that will send a room invite to another user.
     */
    virtual FRoomProviderActionOperationRef CreateSendRoomInviteOperation(
        const FRoomUserId &UserId,
        const FRoomRef &Room,
        const FRoomUserId &TargetUserId) = 0;

    /**
     * The callback type for send room invite operation.
     */
    typedef TDelegate<void(FError ErrorCode)> FOnExecuteSendRoomInviteOperationComplete;

    /**
     * Called by the room system to send a room invite to another user.
     */
    virtual void ExecuteSendRoomInviteOperation(
        const FRoomProviderActionOperationRef &SendOperation,
        const FOnExecuteSendRoomInviteOperationComplete &OnComplete) = 0;

    /**
     * Create an operation that is only used to read the state of the underlying room without making
     * changes. This operation is constructed by SynchroniseRooms, but does not have a corresponding
     * Execute function since there are never any changes to apply.
     */
    virtual FRoomProviderReadOperationRef CreateReadRoomOperation(const FRoomUserId &UserId, const FRoomRef &Room) = 0;

public:
    /**
     * Create a room with this room provider.
     */
    void CreateRoom(const FCreateRoomRequest &Request, const IRoomSystem::FOnCreateRoomComplete &OnComplete);

private:
    void UpdateRoomInternal(
        const FRoomRef &Room,
        const FUpdateRoomRequest &Request,
        const IRoomSystem::FOnUpdateRoomComplete &OnComplete,
        const FString &DumpOperationContext);

public:
    /**
     * Update an existing room with this room provider.
     */
    void UpdateRoom(
        const FRoomRef &Room,
        const FUpdateRoomRequest &Request,
        const IRoomSystem::FOnUpdateRoomComplete &OnComplete);

private:
    /**
     * This extended function is used to perform retries for durable searches. For non-durable searches, this function
     * is still called, but with a remaining retry count of 0.
     */
    void SearchRoomsWithRemainingRetryCount(
        const FSearchRoomsRequest &Request,
        const IRoomSystem::FOnSearchRoomsComplete &OnComplete,
        int32 RemainingRetryCount);

protected:
    /**
     * Attempts to convert a read operation into a room search result, reading all the feature states. This is callable
     * by downstream FRoomProvider implementations, as they will need to call this function in order to implement
     * OnRoomInviteReceivedFromSDKEvent.
     */
    FRoomSearchResultPtr CreateRoomSearchResultFromReadOperation(
        FRoomUserId BoundForLocalUser,
        const FRoomProviderReadOperationRef &ReadOp,
        const FString &OperationName,
        const FString &OperationContext,
        const TOptional<EOS_UI_EventId> &AssociatedUiEventId);

public:
    /**
     * Search for rooms with this room provider.
     */
    void SearchRooms(const FSearchRoomsRequest &Request, const IRoomSystem::FOnSearchRoomsComplete &OnComplete);

    /**
     * Join an existing room with this room provider.
     */
    void JoinRoom(const FJoinRoomRequest &Request, const IRoomSystem::FOnJoinRoomComplete &OnComplete);

    /**
     * Leave a room previously created or joined with this room provider.
     */
    void LeaveRoom(
        const FRoomRef &Room,
        const FLeaveRoomRequest &Request,
        const IRoomSystem::FOnLeaveRoomComplete &OnComplete);

    /**
     * Promote another member to be the owner of the room.
     */
    void PromoteRoomMember(
        const FRoomRef &Room,
        const FPromoteRoomMemberRequest &Request,
        const IRoomSystem::FOnPromoteRoomMemberComplete &OnComplete);

    /**
     * Kick another member from the room.
     */
    void KickRoomMember(
        const FRoomRef &Room,
        const FKickRoomMemberRequest &Request,
        const IRoomSystem::FOnKickRoomMemberComplete &OnComplete);

    /**
     * Send a room invite to another user.
     */
    void SendRoomInvite(
        const FRoomRef &Room,
        const FSendRoomInviteRequest &Request,
        const IRoomSystem::FOnSendRoomInviteComplete &OnComplete);

    /**
     * Restore existing room invites and raise the OnRoomInviteReceivedFromSDKEvent event for each room invite restored.
     */
    virtual void RestoreRoomInvites(
        const FRestoreRoomInvitesRequest &Request,
        const IRoomSystem::FOnRestoreRoomInvitesComplete &OnComplete) = 0;

    /**
     * Reject a previously received room invite. To accept a room invite, use JoinRoom with the search result instead.
     */
    virtual void RejectRoomInvite(
        const FRoomInviteRef &RoomInvite,
        const IRoomSystem::FOnRejectRoomInviteComplete &OnComplete) = 0;

    /**
     * The event type when a room join is requested via an EOS SDK event.
     */
    typedef TDelegate<void(FRoomUserId ActorUserId, FRoomSearchResultRef RoomSearchResult)>
        FOnRoomJoinRequestedFromSDKEvent;

    /**
     * An event that the room system will bind to, to be fired when the EOS SDK reports that
     * a room should be joined via some external event (such as a user joining via an overlay).
     */
    virtual FOnRoomJoinRequestedFromSDKEvent &OnRoomJoinRequestedFromSDKEvent() = 0;

    /**
     * The event type when a room leave is requested via an EOS SDK event.
     */
    typedef TDelegate<void(FRoomUserId ActorUserId, FRoomIdRef RoomId)> FOnRoomLeaveRequestedFromSDKEvent;

    /**
     * An event that the room system will bind to, to be fired when the EOS SDK reports that
     * a room has been left via some external event (such as a user leaving via an overlay).
     */
    virtual FOnRoomLeaveRequestedFromSDKEvent &OnRoomLeaveRequestedFromSDKEvent() = 0;

    // @note: There is no need for OnRoomUpdatedFromSDKEvent, because we let SynchroniseRoom
    // discover updates and compute differences, rather than trusting the SDK. We only need
    // join/leave because these are events that the room system can't discover through
    // synchronisation.

    /**
     * The event type when a room invite is received via an EOS SDK event.
     */
    typedef TDelegate<void(
        FRoomInviteId RoomInviteId,
        FRoomUserId RoomInviteSenderUserId,
        FRoomUserId RoomInviteLocalRecipientUserId,
        FRoomSearchResultRef RoomSearchResult)>
        FOnRoomInviteLocallyReceivedFromSDKEvent;

    /**
     * An event that the room system will bind to, to be fired when the EOS SDK reports that a new room invite has been
     * received via some external event.
     */
    virtual FOnRoomInviteLocallyReceivedFromSDKEvent &OnRoomInviteLocallyReceivedFromSDKEvent() = 0;

    /**
     * The event type when a previously received room invite has been accepted via the overlay, and the room system
     * needs to join the target room.
     */
    typedef TDelegate<void(FRoomInviteId RoomInviteId, FRoomUserId RoomInviteLocalRecipientUserId)>
        FOnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent;

    /**
     * An event that the room system will bind to, to be fired when the EOS SDK reports that a previously received room
     * invite has been accepted by the local user and the room system needs to perform a join room operation.
     */
    virtual FOnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent &
    OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent() = 0;

    /**
     * The event type when a previously received room invite has been fully rejected via the overlay, and the room
     * system only needs to remove the invite previously tracked in the receive list. The room system does not need to
     * reject the invite as part of handling this event.
     */
    typedef TDelegate<void(FRoomInviteId RoomInviteId, FRoomUserId RoomInviteLocalRecipientUserId)>
        FOnRoomInviteLocallyFullyRejectedFromSDKEvent;

    /**
     * An event that the room system will bind to, to be fired when the EOS SDK reports that a previously received room
     * invite has been fully rejected via the overlay, and the room system only needs to remove the invite previously
     * tracked in the receive list. The room system does not need to reject the invite as part of handling this event.
     */
    virtual FOnRoomInviteLocallyFullyRejectedFromSDKEvent &OnRoomInviteLocallyFullyRejectedFromSDKEvent() = 0;

private:
    /**
     * Formats a feature attribute key into an attribute key that can be passed to the SDK.
     */
    FString FormatFeatureAttributeKey(const FName &InFeatureName, const FString &InAttributeKey) const;

    /**
     * Attempts to read the feature names from the room attributes using the Redpoint:EOS:FeatureNames attribute.
     */
    TTuple<TSet<FName>, bool> ParseFeatureNamesFromRoomAttributes(
        const TMap<FString, FRoomAttribute> &AttributesByKey) const;

    /**
     * Iterate through the attributes map and bucket them into non-feature room attributes and attributes by feature
     * name.
     */
    void BucketRoomAttributesIntoNonFeatureAndFeatureAttributes(
        const TMap<FString, FRoomAttribute> &AttributesByKey,
        TMap<FString, FRoomAttribute> &NonFeatureAttributes,
        TMap<FName, TMap<FString, FRoomAttribute>> &FeatureAttributes) const;

protected:
    /**
     * Validates the namespace attributes and returns the room namespace if valid.
     */
    TOptional<FName> GetRoomNamespaceFromAttributesByKey(const TMap<FString, FRoomAttribute> &AttributesByKey) const;

private:
    /**
     * Synchronises a single room tracked by this provider.
     */
    void SynchroniseRoom(
        const FRoomRef &InRoom,
        const Synchronisation::FSynchronisationContextRef InSynchronisationContext,
        bool bInitialHostSync);

public:
    /**
     * Called every frame by the room system to synchronise all of the rooms known
     * to the room system with their underlying EOS SDK state, generating events in Unreal
     * Engine based on the observed changes.
     */
    void SynchroniseRooms(
        const TArray<FRoomRef> &InRooms,
        const Synchronisation::FSynchronisationContextRef InSynchronisationContext);

    /**
     * Resets the entire state of the room provider to nothing; called during shutdown of a platform handle.
     */
    virtual void Reset();
};

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(3254663568, Redpoint::EOS::Rooms::Providers, FRoomProvider)
}

REDPOINT_EOS_CODE_GUARD_END()