// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/RoomProvider.h"

#include "Containers/Ticker.h"
#include "RedpointEOSAPI/Lobby/NotifyLobbyMemberStatusReceived.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSAPI/UI/AcknowledgeEventId.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/SetArray.h"
#include "RedpointEOSRooms/Logging.h"
#include "RedpointEOSRooms/Providers/RoomImpl.h"
#include "RedpointEOSRooms/Providers/RoomSearchResultImpl.h"
#include "RedpointEOSRooms/Synchronisation/RoomAttributeSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomFeatureSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomMaxMembersSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomMemberAttributeSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomMemberInfoSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomMemberSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/RoomOwnerSynchronisationDifference.h"
#include "RedpointEOSRooms/Synchronisation/SynchronisationContextNull.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(Operation, ErrorFunction, ...)                                         \
    {                                                                                                                  \
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);                                                                     \
                                                                                                                       \
        auto _SingleOpError = (Operation);                                                                             \
        if (!_SingleOpError.WasSuccessful())                                                                           \
        {                                                                                                              \
            ErrorFunction(_SingleOpError, ##__VA_ARGS__);                                                              \
            return;                                                                                                    \
        }                                                                                                              \
    }

#define ATTEMPT_SET_WITH_DIRECT_ERROR_RETURN(Operation)                                                                \
    {                                                                                                                  \
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);                                                                     \
                                                                                                                       \
        auto _SingleOpError = (Operation);                                                                             \
        if (!_SingleOpError.WasSuccessful())                                                                           \
        {                                                                                                              \
            return _SingleOpError;                                                                                     \
        }                                                                                                              \
    }

namespace REDPOINT_EOS_FILE_NS_ID(3254663568, Redpoint::EOS::Rooms::Providers)
{
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::Rooms::Synchronisation;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Core::Utils;

FRoomProvider::FRoomProvider(const FPlatformHandle &InBasePlatformHandle)
    : BasePlatformHandle(InBasePlatformHandle)
    , FeatureHandlers()
    , FeatureSetHandlers()
    , AutomaticRoomUpdatesInProgress()
    , DeferredAutomaticRoomUpdates()
{
}

const TMap<FName, TSharedPtr<FRoomProviderFeatureHandlerBase>> &FRoomProvider::GetFeatureHandlers() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->FeatureHandlers;
}

void FRoomProvider::CreateRoom(const FCreateRoomRequest &Request, const IRoomSystem::FOnCreateRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    // Check that the request is valid.
    if (!IsProductUserId(Request.LocalUser))
    {
        OnComplete.ExecuteIfBound(
            Redpoint::EOS::API::Errors::InvalidUser(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Invalid local user ID passed to CreateRoom.")),
            nullptr);
        return;
    }
    for (const auto &FeatureRequestName : Request.GetFeatureRequestNames())
    {
        // Is this feature allowed for room creation?
        auto FeatureHandler = this->FeatureHandlers.Find(FeatureRequestName);
        if (FeatureHandler == nullptr || !(*FeatureHandler)->AllowOnCreate())
        {
            OnComplete.ExecuteIfBound(
                Redpoint::EOS::API::Errors::InvalidParameters(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(
                        TEXT("The room feature '%s' is not supported for CreateRoom on this room provider."),
                        *FeatureRequestName.ToString())),
                nullptr);
            return;
        }

        // If the feature request is not valid, error.
        auto RequestFeature = Request.GetFeatureRequestBase(FeatureRequestName);
        if (!RequestFeature->IsValid())
        {
            OnComplete.ExecuteIfBound(
                Redpoint::EOS::API::Errors::InvalidParameters(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(
                        TEXT("The room feature request '%s' is not in a valid state."),
                        *FeatureRequestName.ToString())),
                nullptr);
            return;
        }
    }

    // Create the create operation.
    auto CreateOp =
        this->InitializeOperation(this->CreateCreateRoomOperation(Request.LocalUser, Request.RoomNamespace));

    // Apply all of the features to the operation using their handlers.
    TMap<FName, TSharedRef<IRoomFeatureRequestBase>> FeatureRequestsForSetHandlers;
    for (const auto &FeatureRequestName : Request.GetFeatureRequestNames())
    {
        // Run the custom handler for the feature request if one is set.
        auto FeatureRequest = Request.GetFeatureRequestBase(FeatureRequestName);
        auto FeatureHandler = this->FeatureHandlers.FindChecked(FeatureRequestName);
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            FeatureHandler->ApplyFeatureRequestOnCreateRoomOperation().Execute(CreateOp, FeatureRequest.ToSharedRef()),
            OnComplete.ExecuteIfBound,
            nullptr);

        // Serialize the feature request to attributes and apply them.
        for (const auto &KV :
             FeatureHandler->SerializeFeatureRequestToRoomAttributes().Execute(CreateOp, FeatureRequest.ToSharedRef()))
        {
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                CreateOp->AddAttribute(this->FormatFeatureAttributeKey(FeatureRequestName, KV.Key), KV.Value),
                OnComplete.ExecuteIfBound,
                nullptr);
        }
        for (const auto &KV : FeatureHandler->SerializeFeatureRequestToRoomMemberAttributes().Execute(
                 CreateOp,
                 FeatureRequest.ToSharedRef()))
        {
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                CreateOp->AddMemberAttribute(this->FormatFeatureAttributeKey(FeatureRequestName, KV.Key), KV.Value),
                OnComplete.ExecuteIfBound,
                nullptr);
        }

        FeatureRequestsForSetHandlers.Add(FeatureRequestName, FeatureRequest.ToSharedRef());
    }
    for (const auto &KV : this->FeatureHandlers)
    {
        if (!Request.GetFeatureRequestBase(KV.Key).IsValid())
        {
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                KV.Value->ApplyOmittedFeatureRequestOnCreateRoomOperation().Execute(CreateOp),
                OnComplete.ExecuteIfBound,
                nullptr);
        }
    }
    for (const auto &SetHandler : this->FeatureSetHandlers)
    {
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            SetHandler->ApplyFeatureRequestSetOnCreateRoomOperation()
                .Execute(CreateOp, FeatureRequestsForSetHandlers, TSet<FName>()),
            OnComplete.ExecuteIfBound,
            nullptr);
    }

    // Apply basic settings to the operation.
    ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
        CreateOp->SetMaxMembers(Request.MaxMemberCount),
        OnComplete.ExecuteIfBound,
        nullptr);
    for (const auto &KV : Request.Attributes)
    {
        if (KV.Key.StartsWith(ReservedRoomAttributePrefix, ESearchCase::IgnoreCase))
        {
            OnComplete.ExecuteIfBound(
                Redpoint::EOS::API::Errors::InvalidParameters(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(
                        TEXT("You can not set a room attribute that starts with '%s' as it is a reserved prefix."),
                        *ReservedRoomAttributePrefix)),
                nullptr);
            return;
        }
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            CreateOp->AddAttribute(KV.Key, KV.Value),
            OnComplete.ExecuteIfBound,
            nullptr);
    }
    for (const auto &KV : Request.MemberAttributes)
    {
        if (KV.Key.StartsWith(ReservedRoomAttributePrefix, ESearchCase::IgnoreCase))
        {
            OnComplete.ExecuteIfBound(
                Redpoint::EOS::API::Errors::InvalidParameters(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(
                        TEXT("You can not set a room member attribute that starts with '%s' as it is a reserved "
                             "prefix."),
                        *ReservedRoomAttributePrefix)),
                nullptr);
            return;
        }
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            CreateOp->AddMemberAttribute(KV.Key, KV.Value),
            OnComplete.ExecuteIfBound,
            nullptr);
    }

    // Execute the first phase of the create room operation.
    this->ExecuteCreateRoomOperation(
#if !NO_LOGGING
        this->DumpOperation(CreateOp, TEXT("CreateRoom"), TEXT("first phase of CreateRoom call")),
#else
        CreateOp,
#endif
        FOnExecuteCreateRoomOperationComplete::CreateSPLambda(
            this,
            [this, Request, OnComplete](FError ErrorCode, FRoomProviderWriteOperationPtr PostCreateOp) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                // Check that the first phase of room creation succeeded.
                if (!ErrorCode.WasSuccessful())
                {
                    OnComplete.ExecuteIfBound(ErrorCode, nullptr);
                    return;
                }
                checkf(PostCreateOp.IsValid(), TEXT("PostCreateOp must be valid if ErrorCode is successful."));

#if !NO_LOGGING
                if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
                {
                    // Ensure we initialize the debug JSON since it's not the implementors responsibility.
                    PostCreateOp->InitializeDebugJson();
                }
#endif

                // Define the clean up logic we need to run if we're going to stop with a
                // failure after this point.
                auto CleanupRoomAndReturn = [this, OnComplete, PostCreateOp](const FError &PostCreateErrorCode) {
                    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                    this->CancelPostCreateRoomOperation(
                        PostCreateOp.ToSharedRef(),
                        FOnCancelPostCreateRoomOperationComplete::CreateSPLambda(
                            this,
                            [OnComplete, PostCreateErrorCode](FError CleanupErrorCode) {
                                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                                OnComplete.ExecuteIfBound(
                                    !CleanupErrorCode.WasSuccessful() ? CleanupErrorCode : PostCreateErrorCode,
                                    nullptr);
                                return;
                            }));
                };

                // Apply all of the features to the post-create operation using their handlers.
                TMap<FName, TSharedRef<IRoomFeatureRequestBase>> FeatureRequestsForSetHandlersPostCreate;
                TSet<FString> FeatureNamesSet;
                for (const auto &FeatureRequestName : Request.GetFeatureRequestNames())
                {
                    // Run the custom handler for the feature request if one is set.
                    auto FeatureRequest = Request.GetFeatureRequestBase(FeatureRequestName);
                    auto FeatureHandler = this->FeatureHandlers.FindChecked(FeatureRequestName);
                    ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                        FeatureHandler->ApplyFeatureRequestOnPostCreateRoomOperation().Execute(
                            PostCreateOp.ToSharedRef(),
                            FeatureRequest.ToSharedRef()),
                        CleanupRoomAndReturn);

                    // Serialize the feature request to attributes and apply them.
                    for (const auto &KV : FeatureHandler->SerializeFeatureRequestToRoomAttributes().Execute(
                             PostCreateOp.ToSharedRef(),
                             FeatureRequest.ToSharedRef()))
                    {
                        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                            PostCreateOp->AddAttribute(
                                this->FormatFeatureAttributeKey(FeatureRequestName, KV.Key),
                                KV.Value),
                            CleanupRoomAndReturn);
                    }
                    for (const auto &KV : FeatureHandler->SerializeFeatureRequestToRoomMemberAttributes().Execute(
                             PostCreateOp.ToSharedRef(),
                             FeatureRequest.ToSharedRef()))
                    {
                        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                            PostCreateOp->AddMemberAttribute(
                                this->FormatFeatureAttributeKey(FeatureRequestName, KV.Key),
                                KV.Value),
                            CleanupRoomAndReturn);
                    }

                    FeatureRequestsForSetHandlersPostCreate.Add(FeatureRequestName, FeatureRequest.ToSharedRef());
                    FeatureNamesSet.Add(FeatureRequestName.ToString());
                }
                for (const auto &KV : this->FeatureHandlers)
                {
                    if (!Request.GetFeatureRequestBase(KV.Key).IsValid())
                    {
                        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                            KV.Value->ApplyOmittedFeatureRequestOnPostCreateRoomOperation().Execute(
                                PostCreateOp.ToSharedRef()),
                            CleanupRoomAndReturn);
                    }
                }
                for (const auto &SetHandler : this->FeatureSetHandlers)
                {
                    ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                        SetHandler->ApplyFeatureRequestSetOnPostCreateRoomOperation().Execute(
                            PostCreateOp.ToSharedRef(),
                            FeatureRequestsForSetHandlersPostCreate,
                            TSet<FName>()),
                        CleanupRoomAndReturn);
                }

                // Apply basic settings to the operation.
                ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                    PostCreateOp->SetMaxMembers(Request.MaxMemberCount),
                    CleanupRoomAndReturn);
                for (const auto &KV : Request.Attributes)
                {
                    ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                        PostCreateOp->AddAttribute(KV.Key, KV.Value),
                        CleanupRoomAndReturn);
                }
                for (const auto &KV : Request.MemberAttributes)
                {
                    ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                        PostCreateOp->AddMemberAttribute(KV.Key, KV.Value),
                        CleanupRoomAndReturn);
                }

                // Set the feature names and mark the room as ready.
                ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                    PostCreateOp->AddAttribute(ReadyRoomAttribute, true),
                    CleanupRoomAndReturn);
                ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                    PostCreateOp->AddMemberAttribute(ReadyRoomMemberAttribute, true),
                    CleanupRoomAndReturn);
                ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                    PostCreateOp->AddAttribute(FeatureNamesRoomAttribute, SetToArray(FeatureNamesSet)),
                    CleanupRoomAndReturn);
                ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                    PostCreateOp->AddAttribute(
                        NamespaceFilterRoomAttribute,
                        Request.RoomNamespace.ToString().ToUpper()),
                    CleanupRoomAndReturn);
                ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                    PostCreateOp->AddAttribute(NamespaceCasingRoomAttribute, Request.RoomNamespace.ToString()),
                    CleanupRoomAndReturn);

                // Execute the second phase of the room create operation.
                this->ExecutePostCreateRoomOperation(
#if !NO_LOGGING
                    this->DumpOperation(
                        PostCreateOp.ToSharedRef(),
                        TEXT("PostCreateRoom"),
                        TEXT("second phase of CreateRoom call")),
#else
                    PostCreateOp.ToSharedRef(),
#endif
                    FOnExecutePostCreateRoomOperationComplete::CreateSPLambda(
                        this,
                        [this, OnComplete, PostCreateOp, CleanupRoomAndReturn](
                            FError PostCreateErrorCode,
                            FRoomPtr Room) {
                            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                            // Check that the second phase of room creation succeeded.
                            if (!PostCreateErrorCode.WasSuccessful())
                            {
                                CleanupRoomAndReturn(PostCreateErrorCode);
                                return;
                            }

                            // Ensure we have a room on successful result.
                            checkf(Room.IsValid(), TEXT("Room must be valid if PostCreateErrorCode is successful."));

                            // Synchronise the room based on the state it now has. The state it now has will contain the
                            // serialized state of all the features in the original request, and using the same logic
                            // here as on clients ensures that the owner and clients all see the same state.
                            Synchronisation::FSynchronisationContextRef SynchronisationContextNull =
                                MakeShared<Synchronisation::FSynchronisationContextNull>();
                            this->SynchroniseRoom(Room.ToSharedRef(), SynchronisationContextNull, true);

#if !NO_LOGGING
                            // Dump the created room state, so we can see what it looks like in the logs after the
                            // post-create step has finished. This won't happen as part of SynchroniseRoom above, since
                            // the null synchronisation context discards recorded changes (which SynchroniseRoom uses
                            // to determine if it should dump operation state).
                            if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
                            {
                                auto ReadOp = this->InitializeOperation(
                                    this->CreateReadRoomOperation(PostCreateOp->GetActorUserId(), Room.ToSharedRef()));
                                this->DumpOperation(
                                    ReadOp,
                                    TEXT("PostCreateRoom"),
                                    TEXT("room state after CreateRoom has succeeded"));
                            }
#endif

                            // The room was successfully created.
                            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success(), Room);
                        }));
            }));
}

void FRoomProvider::UpdateRoomInternal(
    const FRoomRef &Room,
    const FUpdateRoomRequest &OriginalRequest,
    const IRoomSystem::FOnUpdateRoomComplete &OnComplete,
    const FString &DumpOperationContext)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    // Ensure that this room is for this provider.
    if (!Room->GetRoomId()->GetProviderName().IsEqual(this->GetProviderName()))
    {
        OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidUser(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Invalid room ID passed to FRoomProvider::UpdateRoom.")));
        return;
    }
    TSharedRef<FRoomImpl> RoomImpl = StaticCastSharedRef<FRoomImpl>(Room);

    // Determine whether or not the user is authoritative for this request i.e.
    // whether they own the room. If they're not authoritative, then we don't make
    // any changes to the room other than member updates.
    bool bAuthoritative = RoomImpl->GetBoundForLocalUser() == RoomImpl->GetRoomOwner()->GetUserId();

    // Check that the request is valid.
    for (const auto &FeatureRequestName : OriginalRequest.GetFeatureRequestNames())
    {
        // Is this feature allowed for room update?
        auto FeatureHandler = this->FeatureHandlers.Find(FeatureRequestName);
        if (FeatureHandler == nullptr)
        {
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidParameters(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("The room feature '%s' is not supported by this room provider."),
                    *FeatureRequestName.ToString())));
            return;
        }
        if (RoomImpl->GetRoomFeatureBase(FeatureRequestName).IsValid() && !(*FeatureHandler)->AllowModifyOnUpdate())
        {
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidParameters(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("The room feature '%s' can not be modified via UpdateRoom on this room provider."),
                    *FeatureRequestName.ToString())));
            return;
        }
        if (!RoomImpl->GetRoomFeatureBase(FeatureRequestName).IsValid() && !(*FeatureHandler)->AllowAddOnUpdate())
        {
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidParameters(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("The room feature '%s' can not be added via UpdateRoom on this room provider."),
                    *FeatureRequestName.ToString())));
            return;
        }

        // If the feature request is not valid, error.
        auto RequestFeature = OriginalRequest.GetFeatureRequestBase(FeatureRequestName);
        if (!RequestFeature->IsValid())
        {
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidParameters(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("The room feature request '%s' is not in a valid state."),
                    *FeatureRequestName.ToString())));
            return;
        }
    }
    for (const auto &FeatureRequestName : OriginalRequest.GetDeleteFeatureNames())
    {
        // Is this feature allowed for room update?
        auto FeatureHandler = this->FeatureHandlers.Find(FeatureRequestName);
        if (FeatureHandler == nullptr || !(*FeatureHandler)->AllowDeleteOnUpdate())
        {
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidParameters(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("The room feature '%s' can not be removed via UpdateRoom on this room provider."),
                    *FeatureRequestName.ToString())));
            return;
        }
    }

    // Create a mutable version of the original request, and backfill any
    // features that the user didn't pass in the update (so that we will reserialize
    // their states).
    FUpdateRoomRequest Request = OriginalRequest;
    TSet<FName> DeleteFeatureNames = Request.GetDeleteFeatureNames();
    for (const auto &ExistingFeature : Room->GetRoomFeatureNames())
    {
        if (!DeleteFeatureNames.Contains(ExistingFeature) && !Request.GetFeatureRequestBase(ExistingFeature).IsValid())
        {
            TSharedPtr<Features::IRoomFeatureRequestBase> ExistingFeatureRequest =
                Room->GetRoomFeatureBase(ExistingFeature)->GetFeatureRequestForUpdate();
            if (ExistingFeatureRequest.IsValid())
            {
                Request.AddFeatureRequest<Features::IRoomFeatureRequestBase>(ExistingFeatureRequest.ToSharedRef());
            }
        }
    }

    // Create the update operation.
    auto UpdateOp = this->InitializeOperation(this->CreateUpdateRoomOperation(RoomImpl->GetBoundForLocalUser(), Room));

    // Apply all of the features to the operation using their handlers, and separately
    // record all of the attributes that the features want to set. We don't call
    // AddAttribute and AddMemberAttribute immediately though, since we need to compute
    // the differential and figure out if any attributes need to be removed.
    TMap<FName, TSharedRef<IRoomFeatureRequestBase>> FeatureRequestsForSetHandlers;
    TMap<FName, TMap<FString, FRoomAttribute>> DesiredFeatureRoomAttributes;
    TMap<FName, TMap<FString, FRoomAttribute>> DesiredFeatureRoomMemberAttributes;
    for (const auto &FeatureRequestName : Request.GetFeatureRequestNames())
    {
        // Run the custom handler for the feature request if one is set.
        auto FeatureRequest = Request.GetFeatureRequestBase(FeatureRequestName);
        auto FeatureHandler = this->FeatureHandlers.FindChecked(FeatureRequestName);
        if (bAuthoritative)
        {
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                FeatureHandler->ApplyFeatureRequestOnUpdateRoomOperation()
                    .Execute(UpdateOp, FeatureRequest, Room->GetRoomFeatureBase(FeatureRequestName)),
                OnComplete.ExecuteIfBound);
        }

        // Serialize the feature request to attributes and apply them.
        if (bAuthoritative)
        {
            DesiredFeatureRoomAttributes.Add(
                FeatureRequestName,
                FeatureHandler->SerializeFeatureRequestToRoomAttributes().Execute(
                    UpdateOp,
                    FeatureRequest.ToSharedRef()));
        }
        DesiredFeatureRoomMemberAttributes.Add(
            FeatureRequestName,
            FeatureHandler->SerializeFeatureRequestToRoomMemberAttributes().Execute(
                UpdateOp,
                FeatureRequest.ToSharedRef()));
        FeatureRequestsForSetHandlers.Add(FeatureRequestName, FeatureRequest.ToSharedRef());
    }
    for (const auto &FeatureRequestName : Request.GetDeleteFeatureNames())
    {
        // Run the custom handler for the feature request if one is set.
        auto FeatureRequest = Request.GetFeatureRequestBase(FeatureRequestName);
        auto FeatureHandler = this->FeatureHandlers.FindChecked(FeatureRequestName);
        auto ExistingFeature = RoomImpl->GetRoomFeatureBase(FeatureRequestName);
        if (bAuthoritative && ExistingFeature.IsValid())
        {
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                FeatureHandler->ApplyFeatureRequestOnUpdateRoomOperation().Execute(UpdateOp, nullptr, ExistingFeature),
                OnComplete.ExecuteIfBound);
        }
    }
    if (bAuthoritative)
    {
        // Build a map of existing features on the room so that the set handler for the
        // update room operation can inspect their state.
        TMap<FName, TSharedRef<IRoomFeature>> ExistingFeatures;
        for (const auto &FeatureName : RoomImpl->GetRoomFeatureNames())
        {
            auto Feature = RoomImpl->GetRoomFeatureBase(FeatureName);
            if (Feature.IsValid())
            {
                ExistingFeatures.Add(FeatureName, Feature.ToSharedRef());
            }
        }

        // Execute all of the set handlers for this update room operation.
        for (const auto &SetHandler : this->FeatureSetHandlers)
        {
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                SetHandler->ApplyFeatureRequestSetOnUpdateRoomOperation().Execute(
                    UpdateOp,
                    FeatureRequestsForSetHandlers,
                    ExistingFeatures,
                    Request.GetDeleteFeatureNames()),
                OnComplete.ExecuteIfBound);
        }
    }

    // Define a reconciler that can get feature attributes in sync, by removing feature attributes that
    // are no longer being serialized (either by the feature, or because the feature was removed).
    auto ReconcileFeatureAttributes = [this](
                                          const TMap<FString, FRoomAttribute> &CurrentRoomAttributes,
                                          TMap<FName, TMap<FString, FRoomAttribute>> &DesiredFeatureRoomAttributesMap,
                                          const TDelegate<FError(FString Key, FRoomAttribute Value)> &SetAttribute,
                                          const TDelegate<FError(FString Key)> &RemoveAttribute) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        TMap<FString, FRoomAttribute> _;
        TMap<FName, TMap<FString, FRoomAttribute>> CurrentFeatureRoomAttributes;
        this->BucketRoomAttributesIntoNonFeatureAndFeatureAttributes(
            CurrentRoomAttributes,
            _,
            CurrentFeatureRoomAttributes);

        // Ensure the desired map contains an empty map value for any features we currently
        // have (even if that feature has been removed in this request and thus has not
        // had a map value created for it during the previous section).
        for (const auto &KV : CurrentFeatureRoomAttributes)
        {
            DesiredFeatureRoomAttributesMap.FindOrAdd(KV.Key, TMap<FString, FRoomAttribute>());
        }

        // For each of our attribute maps for current features, remove any attributes
        // that are no longer present.
        for (const auto &CurrentKV : CurrentFeatureRoomAttributes)
        {
            const auto &CurrentFeatureRoomAttributeMap = CurrentKV.Value;
            const auto &DesiredFeatureRoomAttributeMap = DesiredFeatureRoomAttributesMap.FindChecked(CurrentKV.Key);
            for (const auto &AttrKV : CurrentFeatureRoomAttributeMap)
            {
                if (!DesiredFeatureRoomAttributeMap.Contains(AttrKV.Key))
                {
                    ATTEMPT_SET_WITH_DIRECT_ERROR_RETURN(
                        RemoveAttribute.Execute(this->FormatFeatureAttributeKey(CurrentKV.Key, AttrKV.Key)));
                }
            }
        }

        // For each of our attribute maps for our desired features, set their attributes.
        for (const auto &DesiredKV : DesiredFeatureRoomAttributesMap)
        {
            for (const auto &AttrKV : DesiredKV.Value)
            {
                ATTEMPT_SET_WITH_DIRECT_ERROR_RETURN(
                    SetAttribute.Execute(this->FormatFeatureAttributeKey(DesiredKV.Key, AttrKV.Key), AttrKV.Value));
            }
        }

        return Redpoint::EOS::API::Errors::Success();
    };

    // Reconcile feature room attributes.
    if (bAuthoritative)
    {
        auto CurrentRoomAttributes = UpdateOp->GetAttributes().Key;
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            ReconcileFeatureAttributes(
                CurrentRoomAttributes,
                DesiredFeatureRoomAttributes,
                TDelegate<FError(FString Key, FRoomAttribute Value)>::CreateLambda(
                    [UpdateOp](FString Key, FRoomAttribute Value) {
                        return UpdateOp->AddAttribute(Key, Value);
                    }),
                TDelegate<FError(FString Key)>::CreateLambda([UpdateOp](FString Key) {
                    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                    return UpdateOp->RemoveAttribute(Key);
                })),
            OnComplete.ExecuteIfBound);
    }

    // Reconcile feature room member attributes. Unlike feature room attributes above,
    // this is done even if the user does not have authority.
    {
        auto CurrentRoomMemberAttributes = UpdateOp->GetMemberAttributes(RoomImpl->GetBoundForLocalUser()).Key;
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            ReconcileFeatureAttributes(
                CurrentRoomMemberAttributes,
                DesiredFeatureRoomMemberAttributes,
                TDelegate<FError(FString Key, FRoomAttribute Value)>::CreateLambda(
                    [UpdateOp](FString Key, FRoomAttribute Value) {
                        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                        return UpdateOp->AddMemberAttribute(Key, Value);
                    }),
                TDelegate<FError(FString Key)>::CreateLambda([UpdateOp](FString Key) {
                    return UpdateOp->RemoveMemberAttribute(Key);
                })),
            OnComplete.ExecuteIfBound);
    }

    // @hack: This is a workaround for a bug in the EOS SDK where performing a lobby update
    // as the owner will clear the owner's member attributes if the owner only makes changes
    // to non-member attributes:
    //
    // https://eoshelp.epicgames.com/s/case/500QP000006FXl0YAG/bug-report-updating-lobby-data-without-updating-member-data-from-the-host-causes-previous-host-member-data-to-be-cleared?language=en_US
    // (private case)
    //
    // By always setting the Redpoint:EOS:Ready member attribute on the owner when the owner
    // updates the lobby, we prevent the EOS SDK sending an incorrect `memberData` value in
    // the `lobbydata` request it sends to the backend.
    //
    // We don't need to do this on non-owner members, as they don't use the `lobbydata` request;
    // instead they use `memberdata` which is unaffected by this bug.
    //
    if (bAuthoritative)
    {
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            UpdateOp->AddMemberAttribute(ReadyRoomMemberAttribute, true),
            OnComplete.ExecuteIfBound);
    }

    // @hack: This is a workaround for a bug in the EOS SDK where performing a lobby update as the owner won't be sent
    // out on the WebSocket if the only changes are being made are member attributes:
    //
    // https://eoshelp.epicgames.com/s/case/500QP00000IN4VEYA1/extremely-strange-lobby-attribute-desync-between-two-users-on-two-platforms-in-the-same-eos-sdk-process
    // (private case)
    //
    // By forcing a non-member attribute to always be updated, we ensure that the `lobbydata` request gets sent on the
    // WebSocket.
    //
    if (bAuthoritative)
    {
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            UpdateOp->AddAttribute(
                ForceUpdateIdentRoomAttribute,
                FString::Printf(TEXT("%lld"), FDateTime::UtcNow().GetTicks())),
            OnComplete.ExecuteIfBound);
    }

    // Apply basic settings to the operation.
    if (bAuthoritative)
    {
        if (Request.MaxMemberCount.IsSet())
        {
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                UpdateOp->SetMaxMembers(Request.MaxMemberCount.GetValue()),
                OnComplete.ExecuteIfBound);
        }
        for (const auto &KV : Request.UpsertAttributes)
        {
            if (KV.Key.StartsWith(ReservedRoomAttributePrefix, ESearchCase::IgnoreCase))
            {
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidParameters(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(
                        TEXT("You can not add or update a room attribute that starts with '%s' as it is a reserved "
                             "prefix."),
                        *ReservedRoomAttributePrefix)));
                return;
            }
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                UpdateOp->AddAttribute(KV.Key, KV.Value),
                OnComplete.ExecuteIfBound);
        }
        for (const auto &Key : Request.DeleteAttributes)
        {
            if (Key.StartsWith(ReservedRoomAttributePrefix, ESearchCase::IgnoreCase))
            {
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidParameters(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(
                        TEXT("You can not remove a room attribute that starts with '%s' as it is a reserved prefix."),
                        *ReservedRoomAttributePrefix)));
                return;
            }
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(UpdateOp->RemoveAttribute(Key), OnComplete.ExecuteIfBound);
        }
    }
    else
    {
        if (Request.MaxMemberCount.IsSet())
        {
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::AccessDenied(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("You can not set the maximum member count on this room as you are not the owner.")));
            return;
        }
        if (Request.UpsertAttributes.Num() > 0)
        {
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::AccessDenied(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("You can not upsert non-member attributes on the room as you are not the owner.")));
            return;
        }
        if (Request.DeleteAttributes.Num() > 0)
        {
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::AccessDenied(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("You can not delete non-member attributes on the room as you are not the owner.")));
            return;
        }
    }
    for (const auto &KV : Request.UpsertMemberAttributes)
    {
        if (KV.Key.StartsWith(ReservedRoomAttributePrefix, ESearchCase::IgnoreCase))
        {
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidParameters(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("You can not add or update a room member attribute that starts with '%s' as it is a reserved "
                         "prefix."),
                    *ReservedRoomAttributePrefix)));
            return;
        }
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            UpdateOp->AddMemberAttribute(KV.Key, KV.Value),
            OnComplete.ExecuteIfBound);
    }
    for (const auto &Key : Request.DeleteMemberAttributes)
    {
        if (Key.StartsWith(ReservedRoomAttributePrefix, ESearchCase::IgnoreCase))
        {
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidParameters(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("You can not remove a room member attribute that starts with '%s' as it is a reserved "
                         "prefix."),
                    *ReservedRoomAttributePrefix)));
            return;
        }
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(UpdateOp->RemoveMemberAttribute(Key), OnComplete.ExecuteIfBound);
    }

    // Execute the update room operation.
    this->ExecuteUpdateRoomOperation(
#if !NO_LOGGING
        this->DumpOperation(UpdateOp, TEXT("UpdateRoom"), DumpOperationContext),
#else
        UpdateOp,
#endif
        FOnExecuteUpdateRoomOperationComplete::CreateSPLambda(this, [RoomImpl, Request, OnComplete](FError ErrorCode) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            // Check that the operation was successful.
            if (!ErrorCode.WasSuccessful())
            {
                OnComplete.ExecuteIfBound(ErrorCode);
                return;
            }

            // @note: We do not immediately synchronise inside the provider like in CreateRoom. Instead,
            // the room system itself does the synchronisation so that OnRoomUpdated event can be raised
            // correctly.

            // Return the success result.
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
        }));
}

void FRoomProvider::UpdateRoom(
    const FRoomRef &Room,
    const FUpdateRoomRequest &Request,
    const IRoomSystem::FOnUpdateRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    this->UpdateRoomInternal(Room, Request, OnComplete, TEXT("UpdateRoom call"));
}

void FRoomProvider::SearchRoomsWithRemainingRetryCount(
    const FSearchRoomsRequest &Request,
    const IRoomSystem::FOnSearchRoomsComplete &OnComplete,
    int32 RemainingRetryCount)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    // Check that the request is valid.
    if (!IsProductUserId(Request.LocalUser))
    {
        OnComplete.ExecuteIfBound(
            Redpoint::EOS::API::Errors::InvalidUser(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Invalid local user ID passed to SearchRooms.")),
            TArray<FRoomSearchResultRef>());
        return;
    }

    // Create the search operation.
    auto SearchOp =
        this->InitializeOperation(this->CreateSearchRoomsOperation(Request.LocalUser, Request.MaxResultCount));

    // Apply search constraints based on request.
    if (Request.RequireRoomId.IsValid())
    {
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            SearchOp->WithRoomId(Request.RequireRoomId.ToSharedRef()),
            OnComplete.ExecuteIfBound,
            TArray<FRoomSearchResultRef>());
    }
    else if (Request.RequireRoomMemberId.IsSet())
    {
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            SearchOp->WithRoomMemberId(Request.RequireRoomMemberId.GetValue()),
            OnComplete.ExecuteIfBound,
            TArray<FRoomSearchResultRef>());
    }
    else
    {
        for (const auto &KV : Request.AttributeConstraints)
        {
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                SearchOp->WithAttributeConstraint(KV.Key, KV.Value),
                OnComplete.ExecuteIfBound,
                TArray<FRoomSearchResultRef>());
        }

        if (!Request.RoomNamespace.IsEqual(NAME_All))
        {
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                SearchOp->WithAttributeConstraint(
                    NamespaceFilterRoomAttribute,
                    FRoomAttributeConstraint(
                        Request.RoomNamespace.ToString().ToUpper(),
                        FRoomAttributeConstraint::EConstraintType::Equal)),
                OnComplete.ExecuteIfBound,
                TArray<FRoomSearchResultRef>());
        }
    }

    // Execute the search operation.
    this->ExecuteSearchRoomsOperation(
#if !NO_LOGGING
        this->DumpOperation(SearchOp, TEXT("SearchRooms"), TEXT("SearchRooms call")),
#else
        SearchOp,
#endif
        FOnExecuteSearchRoomsOperationComplete::CreateSPLambda(
            this,
            [this, Request, OnComplete, RemainingRetryCount](
                FError ErrorCode,
                TArray<FRoomProviderReadOperationRef> ReadOps) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                // Check that the first phase of room creation succeeded.
                if (!ErrorCode.WasSuccessful())
                {
                    OnComplete.ExecuteIfBound(ErrorCode, TArray<FRoomSearchResultRef>());
                    return;
                }

                // If we have no read operations, and we have remaining retries, attempt to search again after a delay.
                if (ReadOps.Num() == 0 && RemainingRetryCount > 0)
                {
                    // Schedule the retry in 1 second.
                    FTSTicker::GetCoreTicker().AddTicker(
                        FTickerDelegate::CreateSPLambda(
                            this,
                            [this, Request, OnComplete, RemainingRetryCount](float DeltaSeconds) {
                                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                                // @note: We decrement the retry count here.
                                UE_LOG(
                                    LogRedpointEOSRooms,
                                    VeryVerbose,
                                    TEXT("Automatically retrying durable search with %d retries remaining..."),
                                    RemainingRetryCount - 1);
                                this->SearchRoomsWithRemainingRetryCount(Request, OnComplete, RemainingRetryCount - 1);
                                return false;
                            }),
                        1.0f);
                    return;
                }

                // For each read operation, we need to construct the search result.
                TArray<FRoomSearchResultRef> Results;
                int ReadOpIndex = 0;
                for (const auto &ReadOp : ReadOps)
                {
                    // Increment the index as we iterate through read operations.
                    ReadOpIndex++;

                    // Convert the read operation to a search result. CreateRoomSearchResultFromReadOperation performs
                    // appropriate logging on failure.
                    auto RoomSearchResult = this->CreateRoomSearchResultFromReadOperation(
                        Request.LocalUser,
                        ReadOp,
                        TEXT("SearchRooms(Result)"),
                        FString::Printf(TEXT("search result %d"), ReadOpIndex),
                        TOptional<EOS_UI_EventId>());
                    if (RoomSearchResult.IsValid())
                    {
                        if (Request.RoomNamespace.IsEqual(NAME_All) ||
                            RoomSearchResult->GetRoomNamespace().IsEqual(Request.RoomNamespace))
                        {
                            Results.Add(RoomSearchResult.ToSharedRef());
                        }
                        else
                        {
                            UE_LOG(
                                LogRedpointEOSRooms,
                                Error,
                                TEXT("%s"),
                                *Redpoint::EOS::API::Errors::NotFound(
                                     ANSI_TO_TCHAR(__FUNCTION__),
                                     FString::Printf(
                                         TEXT("The room search result for room ID '%s' has room namespace '%s', but "
                                              "the namespace specified on the room search request was '%s'. This "
                                              "search result will not be returned from the search operation, as the "
                                              "namespace conflicts. The room namespace must match, even if you are "
                                              "searching for a room by room ID or room member ID."),
                                         *RoomSearchResult->GetRoomId()->ToString(),
                                         *RoomSearchResult->GetRoomNamespace().ToString(),
                                         *Request.RoomNamespace.ToString()))
                                     .ToLogString());
                        }
                    }
                }
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success(), Results);
            }));
}

FRoomSearchResultPtr FRoomProvider::CreateRoomSearchResultFromReadOperation(
    FRoomUserId BoundForLocalUser,
    const FRoomProviderReadOperationRef &ReadOp,
    const FString &OperationName,
    const FString &OperationContext,
    const TOptional<EOS_UI_EventId> &AssociatedUiEventId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

#if !NO_LOGGING
    if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
    {
        // Initialize the debug JSON for the search result.
        ReadOp->InitializeDebugJson();
    }
#endif

    // Read the search result info.
    auto [Info, InfoError] = ReadOp->GetInfo();
    if (!InfoError.WasSuccessful())
    {
#if !NO_LOGGING
        this->DumpOperation(ReadOp, OperationName, OperationContext);
#endif
        UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *InfoError.ToLogString());
        return nullptr;
    }

    // Read the search result attributes.
    auto [AttributesByKey, AttributesByKeyError] = ReadOp->GetAttributes();
    if (!AttributesByKeyError.WasSuccessful())
    {
#if !NO_LOGGING
        this->DumpOperation(ReadOp, OperationName, OperationContext);
#endif
        UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *AttributesByKeyError.ToLogString());
        return nullptr;
    }

    // Parse the feature names.
    auto [FeatureNames, bFeatureNamesWasSuccessful] = this->ParseFeatureNamesFromRoomAttributes(AttributesByKey);
    if (!bFeatureNamesWasSuccessful)
    {
#if !NO_LOGGING
        this->DumpOperation(ReadOp, OperationName, OperationContext);
#endif
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("%s"),
            *Redpoint::EOS::API::Errors::NotFound(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 TEXT("Unable to parse a list of features from this room search result's attributes, "
                      "so this search result will be ignored."))
                 .ToLogString());
        return nullptr;
    }

    // Compute which features for whom we determine presence from the local room state and
    // not the FeatureNames attribute, and thus which feature names should be ignored when
    // parsing search result features.
    TSet<FName> FeaturePresenceDeterminedFromSearchReadOp;
    for (const auto &KV : this->FeatureHandlers)
    {
        if (KV.Value->TestFeaturePresenceFromSearchReadOp().IsBound())
        {
            FeaturePresenceDeterminedFromSearchReadOp.Add(KV.Key);
        }
    }

    // Read all of the room attributes and bucket them into non-feature room attributes
    // and by feature name for the feature search state constructor.
    TMap<FString, FRoomAttribute> NonFeatureAttributes;
    TMap<FName, TMap<FString, FRoomAttribute>> FeatureAttributes;
    this->BucketRoomAttributesIntoNonFeatureAndFeatureAttributes(
        AttributesByKey,
        NonFeatureAttributes,
        FeatureAttributes);

    // For all of the features specified on the room, attempt to construct their search state.
    TMap<FName, TSharedRef<IRoomFeatureSearchStateBase>> RoomFeatureSearchStates;
    for (const auto &FeatureName : FeatureNames)
    {
        bool bConstructed = false;
        const auto *FeatureHandler = this->FeatureHandlers.Find(FeatureName);
        if (FeatureHandler != nullptr)
        {
            if (!FeaturePresenceDeterminedFromSearchReadOp.Contains(FeatureName) ||
                (*FeatureHandler)
                    ->TestFeaturePresenceFromSearchReadOp()
                    .Execute(
                        ReadOp,
                        this->AsShared(),
                        FeatureAttributes.FindOrAdd(FeatureName, TMap<FString, FRoomAttribute>())))
            {
                auto FeatureSearchState =
                    (*FeatureHandler)
                        ->ConstructFeatureSearchState()
                        .Execute(
                            ReadOp,
                            this->AsShared(),
                            FeatureAttributes.FindOrAdd(FeatureName, TMap<FString, FRoomAttribute>()));
                if (FeatureSearchState.IsValid())
                {
                    RoomFeatureSearchStates.Add(FeatureName, FeatureSearchState.ToSharedRef());
                    bConstructed = true;
                }
            }
            if (!bConstructed)
            {
                UE_LOG(
                    LogRedpointEOSRooms,
                    Warning,
                    TEXT("%s"),
                    *Redpoint::EOS::API::Errors::NotFound(
                         ANSI_TO_TCHAR(__FUNCTION__),
                         FString::Printf(
                             TEXT("The room feature '%s' does not have a ConstructFeatureSearchState "
                                  "callback, which means it can not be constructed from search results. "
                                  "The search result will not include this feature, even though it is "
                                  "present in the actual room."),
                             *FeatureName.ToString()))
                         .ToLogString());
            }
        }
    }

    // Determine the room namespace.
    auto RoomNamespace = this->GetRoomNamespaceFromAttributesByKey(AttributesByKey);
    if (!RoomNamespace.IsSet())
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("%s"),
            *Redpoint::EOS::API::Errors::NotFound(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 FString::Printf(
                     TEXT("The room search result for room ID '%s' does not have valid %s and %s attributes, which "
                          "means the room can not be constructed from search results and will be excluded."),
                     *Info->GetRoomId()->ToString(),
                     *NamespaceCasingRoomAttribute,
                     *NamespaceFilterRoomAttribute))
                 .ToLogString());
        return nullptr;
    }

    // Generate and add the search result.
#if !NO_LOGGING
    this->DumpOperation(ReadOp, OperationName, OperationContext);
#endif
    return MakeShared<FRoomSearchResultImpl>(
        BoundForLocalUser,
        Info->GetRoomId(),
        RoomNamespace.GetValue(),
        Info->IsOwnerDedicatedServer() ? MakeShared<FRoomOwner>(Info->GetOwnerDedicatedServerClientId())
                                       : MakeShared<FRoomOwner>(Info->GetOwnerUserId()),
        Info->GetMaxMembers(),
        RoomFeatureSearchStates,
        NonFeatureAttributes,
        ReadOp,
        AssociatedUiEventId);
}

void FRoomProvider::SearchRooms(
    const FSearchRoomsRequest &Request,
    const IRoomSystem::FOnSearchRoomsComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    if (Request.bDurable)
    {
        // Retry up to 9 more times (which makes 10 including the initial request).
        UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("Starting durable search with 9 retries remaining."));
        this->SearchRoomsWithRemainingRetryCount(Request, OnComplete, 9);
    }
    else
    {
        // Automatic retries are not permitted.
        UE_LOG(LogRedpointEOSRooms, VeryVerbose, TEXT("Starting non-durable search."));
        this->SearchRoomsWithRemainingRetryCount(Request, OnComplete, 0);
    }
}

void FRoomProvider::JoinRoom(const FJoinRoomRequest &Request, const IRoomSystem::FOnJoinRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    // Create a wrapper around our completion delegate, so that we can propagate the result code through to the UI event
    // if the join room request has one associated.
    auto CompleteWithUiEvent = [LocalPlatformHandle = this->BasePlatformHandle,
                                Request,
                                OnComplete](const FError &ErrorCode, const FRoomPtr &Room) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        if (Request.SearchResult.IsValid())
        {
            auto UiEventId = Request.SearchResult->GetAssociatedUiEventId();
            if (UiEventId.IsSet())
            {
                using namespace ::Redpoint::EOS::API::UI;
                auto UiResult = FAcknowledgeEventId::Execute(
                    LocalPlatformHandle,
                    FAcknowledgeEventId::Options{UiEventId.GetValue(), ErrorCode.ResultCode()});
                if (UiResult != EOS_EResult::EOS_Success)
                {
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("%s"),
                        *ConvertError(
                             ANSI_TO_TCHAR(__FUNCTION__),
                             TEXT("Failed to acknowledge associated UI event."),
                             UiResult)
                             .ToLogString());
                }
            }
        }
        OnComplete.ExecuteIfBound(ErrorCode, Room);
    };

    // Check that the request is valid.
    if (!IsProductUserId(Request.LocalUser))
    {
        CompleteWithUiEvent(
            Redpoint::EOS::API::Errors::InvalidUser(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Invalid local user ID passed to CreateRoom.")),
            nullptr);
        return;
    }
    for (const auto &FeatureRequestName : Request.GetFeatureRequestNames())
    {
        // Is this feature allowed for room creation?
        auto FeatureHandler = this->FeatureHandlers.Find(FeatureRequestName);
        if (FeatureHandler == nullptr || !(*FeatureHandler)->AllowOnJoin())
        {
            CompleteWithUiEvent(
                Redpoint::EOS::API::Errors::InvalidParameters(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(
                        TEXT("The room feature '%s' is not supported for JoinRoom on this room provider."),
                        *FeatureRequestName.ToString())),
                nullptr);
            return;
        }

        // If the feature request is not valid, error.
        auto RequestFeature = Request.GetFeatureRequestBase(FeatureRequestName);
        if (!RequestFeature->IsValid())
        {
            CompleteWithUiEvent(
                Redpoint::EOS::API::Errors::InvalidParameters(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(
                        TEXT("The room feature request '%s' is not in a valid state."),
                        *FeatureRequestName.ToString())),
                nullptr);
            return;
        }
    }

    // Create the join operation.
    FRoomProviderWriteOperationPtr JoinWriteOp;
    if (Request.RoomId.IsValid())
    {
        JoinWriteOp = this->CreateJoinRoomByIdOperation(Request.LocalUser, Request.RoomId.ToSharedRef());
    }
    else if (Request.SearchResult.IsValid())
    {
        JoinWriteOp =
            this->CreateJoinRoomBySearchResultOperation(Request.LocalUser, Request.SearchResult.ToSharedRef());
    }
    else if (Request.RoomInvite.IsValid())
    {
        JoinWriteOp = this->CreateJoinRoomBySearchResultOperation(
            Request.LocalUser,
            Request.RoomInvite->GetRoomInviteSearchResult());
    }
    else if (Request.RoomRequestedJoin.IsValid())
    {
        JoinWriteOp = this->CreateJoinRoomBySearchResultOperation(
            Request.LocalUser,
            Request.RoomRequestedJoin->GetRoomSearchResult());
    }
    else
    {
        checkf(false, TEXT("Invalid FJoinRoomRequest to call FRoomProvider::JoinRoom on!"));
    }
    auto JoinOp = this->InitializeOperation(JoinWriteOp.ToSharedRef());

    // Apply all of the features to the operation using their handlers.
    TMap<FName, TSharedRef<IRoomFeatureRequestBase>> FeatureRequestsForSetHandlers;
    for (const auto &FeatureRequestName : Request.GetFeatureRequestNames())
    {
        // Run the custom handler for the feature request if one is set.
        auto FeatureRequest = Request.GetFeatureRequestBase(FeatureRequestName);
        auto FeatureHandler = this->FeatureHandlers.FindChecked(FeatureRequestName);
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            FeatureHandler->ApplyFeatureRequestOnJoinRoomOperation().Execute(JoinOp, FeatureRequest.ToSharedRef()),
            CompleteWithUiEvent,
            nullptr);

        // Serialize the feature request to member attributes and apply them.
        for (const auto &KV : FeatureHandler->SerializeFeatureRequestToRoomMemberAttributes().Execute(
                 JoinOp,
                 FeatureRequest.ToSharedRef()))
        {
            ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                JoinOp->AddMemberAttribute(this->FormatFeatureAttributeKey(FeatureRequestName, KV.Key), KV.Value),
                CompleteWithUiEvent,
                nullptr);
        }

        FeatureRequestsForSetHandlers.Add(FeatureRequestName, FeatureRequest.ToSharedRef());
    }
    for (const auto &SetHandler : this->FeatureSetHandlers)
    {
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            SetHandler->ApplyFeatureRequestSetOnJoinRoomOperation()
                .Execute(JoinOp, FeatureRequestsForSetHandlers, TSet<FName>()),
            CompleteWithUiEvent,
            nullptr);
    }

    // Apply basic settings to the operation.
    for (const auto &KV : Request.MemberAttributes)
    {
        if (KV.Key.StartsWith(ReservedRoomAttributePrefix, ESearchCase::IgnoreCase))
        {
            CompleteWithUiEvent(
                Redpoint::EOS::API::Errors::InvalidParameters(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    FString::Printf(
                        TEXT("You can not set a room member attribute that starts with '%s' as it is a reserved "
                             "prefix."),
                        *ReservedRoomAttributePrefix)),
                nullptr);
            return;
        }
        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
            JoinOp->AddMemberAttribute(KV.Key, KV.Value),
            CompleteWithUiEvent,
            nullptr);
    }

    // Execute the first phase of the join operation.
    this->ExecuteJoinRoomOperation(
#if !NO_LOGGING
        this->DumpOperation(JoinOp, TEXT("JoinRoom"), TEXT("first phase of JoinRoom call")),
#else
        JoinOp,
#endif
        FOnExecuteJoinRoomOperationComplete::CreateSPLambda(
            this,
            [this, Request, CompleteWithUiEvent](FError ErrorCode, FRoomProviderWriteOperationPtr PostJoinOp) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                // Check that the first phase of room join succeeded.
                if (!ErrorCode.WasSuccessful())
                {
                    CompleteWithUiEvent(ErrorCode, nullptr);
                    return;
                }
                checkf(PostJoinOp.IsValid(), TEXT("PostJoinOp must be valid if ErrorCode is successful."));

#if !NO_LOGGING
                if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
                {
                    // Ensure we initialize the debug JSON since it's not the implementors responsibility.
                    PostJoinOp->InitializeDebugJson();
                }
#endif

                // Define the clean up logic we need to run if we're going to stop with a
                // failure after this point.
                auto CleanupRoomAndReturn = [this, CompleteWithUiEvent, PostJoinOp](const FError &PostJoinErrorCode) {
                    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                    this->CancelPostJoinRoomOperation(
                        PostJoinOp.ToSharedRef(),
                        FOnCancelPostJoinRoomOperationComplete::CreateSPLambda(
                            this,
                            [CompleteWithUiEvent, PostJoinErrorCode](FError CleanupErrorCode) {
                                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                                CompleteWithUiEvent(
                                    !CleanupErrorCode.WasSuccessful() ? CleanupErrorCode : PostJoinErrorCode,
                                    nullptr);
                                return;
                            }));
                };

                // Apply all of the features to the post-join operation using their handlers.
                TMap<FName, TSharedRef<IRoomFeatureRequestBase>> FeatureRequestsForSetHandlersPostJoin;
                TSet<FString> FeatureNamesSet;
                for (const auto &FeatureRequestName : Request.GetFeatureRequestNames())
                {
                    // Run the custom handler for the feature request if one is set.
                    auto FeatureRequest = Request.GetFeatureRequestBase(FeatureRequestName);
                    auto FeatureHandler = this->FeatureHandlers.FindChecked(FeatureRequestName);
                    ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                        FeatureHandler->ApplyFeatureRequestOnPostJoinRoomOperation().Execute(
                            PostJoinOp.ToSharedRef(),
                            FeatureRequest.ToSharedRef()),
                        CleanupRoomAndReturn);

                    // Serialize the feature request to attributes and apply them.
                    for (const auto &KV : FeatureHandler->SerializeFeatureRequestToRoomMemberAttributes().Execute(
                             PostJoinOp.ToSharedRef(),
                             FeatureRequest.ToSharedRef()))
                    {
                        ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                            PostJoinOp->AddMemberAttribute(
                                this->FormatFeatureAttributeKey(FeatureRequestName, KV.Key),
                                KV.Value),
                            CleanupRoomAndReturn);
                    }

                    FeatureRequestsForSetHandlersPostJoin.Add(FeatureRequestName, FeatureRequest.ToSharedRef());
                    FeatureNamesSet.Add(FeatureRequestName.ToString());
                }
                for (const auto &SetHandler : this->FeatureSetHandlers)
                {
                    ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                        SetHandler->ApplyFeatureRequestSetOnPostJoinRoomOperation()
                            .Execute(PostJoinOp.ToSharedRef(), FeatureRequestsForSetHandlersPostJoin, TSet<FName>()),
                        CleanupRoomAndReturn);
                }

                // Apply basic settings to the operation.
                for (const auto &KV : Request.MemberAttributes)
                {
                    ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                        PostJoinOp->AddMemberAttribute(KV.Key, KV.Value),
                        CleanupRoomAndReturn);
                }

                // Mark the member as ready.
                ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN(
                    PostJoinOp->AddMemberAttribute(ReadyRoomMemberAttribute, true),
                    CleanupRoomAndReturn);

                // Execute the second phase of the room join operation.
                this->ExecutePostJoinRoomOperation(
#if !NO_LOGGING
                    this->DumpOperation(
                        PostJoinOp.ToSharedRef(),
                        TEXT("PostJoinRoom"),
                        TEXT("second phase of JoinRoom call")),
#else
                    PostJoinOp.ToSharedRef(),
#endif
                    FOnExecutePostJoinRoomOperationComplete::CreateSPLambda(
                        this,
                        [this, CompleteWithUiEvent, PostJoinOp, CleanupRoomAndReturn](
                            FError PostJoinErrorCode,
                            FRoomPtr Room) {
                            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                            // Check that the second phase of room join succeeded.
                            if (!PostJoinErrorCode.WasSuccessful())
                            {
                                CleanupRoomAndReturn(PostJoinErrorCode);
                                return;
                            }

                            // Ensure we have a room on successful result.
                            checkf(Room.IsValid(), TEXT("Room must be valid if PostJoinErrorCode is successful."));

                            // Synchronise the room based on the state it now has. The state it now has will contain the
                            // serialized state of all the features in the original request, and using the same logic
                            // here as on clients ensures that the owner and clients all see the same state.
                            Synchronisation::FSynchronisationContextRef SynchronisationContextNull =
                                MakeShared<Synchronisation::FSynchronisationContextNull>();
                            this->SynchroniseRoom(Room.ToSharedRef(), SynchronisationContextNull, true);

#if !NO_LOGGING
                            // Dump the joined room state, so we can see what it looks like in the logs after the
                            // post-join step has finished. This won't happen as part of SynchroniseRoom above, since
                            // the null synchronisation context discards recorded changes (which SynchroniseRoom uses
                            // to determine if it should dump operation state).
                            if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
                            {
                                auto ReadOp = this->InitializeOperation(
                                    this->CreateReadRoomOperation(PostJoinOp->GetActorUserId(), Room.ToSharedRef()));
                                this->DumpOperation(
                                    ReadOp,
                                    TEXT("PostJoinRoom"),
                                    TEXT("room state after JoinRoom has succeeded"));
                            }
#endif

                            // The room was successfully joined.
                            CompleteWithUiEvent(Redpoint::EOS::API::Errors::Success(), Room);
                        }));
            }));
}

void FRoomProvider::LeaveRoom(
    const FRoomRef &Room,
    const FLeaveRoomRequest &Request,
    const IRoomSystem::FOnLeaveRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    // Ensure that this room is for this provider.
    if (!Room->GetRoomId()->GetProviderName().IsEqual(this->GetProviderName()))
    {
        OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidUser(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Invalid room ID passed to FRoomProvider::LeaveRoom.")));
        return;
    }
    TSharedRef<FRoomImpl> RoomImpl = StaticCastSharedRef<FRoomImpl>(Room);

    // Create the leave operation.
    auto LeaveOp =
        this->InitializeOperation(this->CreateLeaveRoomOperation(RoomImpl->GetBoundForLocalUser(), RoomImpl));

    // Execute the leave operation.
    this->ExecuteLeaveRoomOperation(
#if !NO_LOGGING
        this->DumpOperation(LeaveOp, TEXT("LeaveRoom"), TEXT("LeaveRoom call")),
#else
        LeaveOp,
#endif
        FOnExecuteLeaveRoomOperationComplete::CreateSPLambda(this, [OnComplete](FError ErrorCode) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            if (ErrorCode.ResultCode() == EOS_EResult::EOS_Success ||
                ErrorCode.ResultCode() == EOS_EResult::EOS_TimedOut ||
                ErrorCode.ResultCode() == EOS_EResult::EOS_NotFound)
            {
                // These are all success codes, because if the lobby has timed out or
                // otherwise gone away, then we should treat us as having left it successfully.
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
            }
            else
            {
                // In all other cases, report the original error code.
                OnComplete.ExecuteIfBound(ErrorCode);
            }
        }));
}

void FRoomProvider::PromoteRoomMember(
    const FRoomRef &Room,
    const FPromoteRoomMemberRequest &Request,
    const IRoomSystem::FOnPromoteRoomMemberComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    // Ensure that this room is for this provider.
    if (!Room->GetRoomId()->GetProviderName().IsEqual(this->GetProviderName()))
    {
        OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidUser(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Invalid room ID passed to FRoomProvider::PromoteRoomMember.")));
        return;
    }
    TSharedRef<FRoomImpl> RoomImpl = StaticCastSharedRef<FRoomImpl>(Room);

    // Create the promote member operation.
    auto PromoteOp = this->InitializeOperation(
        this->CreatePromoteRoomMemberOperation(RoomImpl->GetBoundForLocalUser(), RoomImpl, Request.TargetUser));

    // Execute the promote operation.
    this->ExecutePromoteRoomMemberOperation(
#if !NO_LOGGING
        this->DumpOperation(PromoteOp, TEXT("PromoteRoomMember"), TEXT("PromoteRoomMember call")),
#else
        PromoteOp,
#endif
        FOnExecutePromoteRoomMemberOperationComplete::CreateSPLambda(this, [OnComplete](FError ErrorCode) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            // Check that the operation was successful.
            if (!ErrorCode.WasSuccessful())
            {
                OnComplete.ExecuteIfBound(ErrorCode);
                return;
            }

            // @note: We do not immediately synchronise inside the provider like in CreateRoom. Instead,
            // the room system itself does the synchronisation so that OnRoomMemberPromoted event can be raised
            // correctly.

            // Return the success result.
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
        }));
}

void FRoomProvider::KickRoomMember(
    const FRoomRef &Room,
    const FKickRoomMemberRequest &Request,
    const IRoomSystem::FOnKickRoomMemberComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    // Ensure that this room is for this provider.
    if (!Room->GetRoomId()->GetProviderName().IsEqual(this->GetProviderName()))
    {
        OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidUser(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Invalid room ID passed to FRoomProvider::KickRoomMember.")));
        return;
    }
    TSharedRef<FRoomImpl> RoomImpl = StaticCastSharedRef<FRoomImpl>(Room);

    // Create the kick member operation.
    auto KickOp = this->InitializeOperation(
        this->CreateKickRoomMemberOperation(RoomImpl->GetBoundForLocalUser(), RoomImpl, Request.TargetUser));

    // Execute the kick operation.
    this->ExecuteKickRoomMemberOperation(
#if !NO_LOGGING
        this->DumpOperation(KickOp, TEXT("KickRoomMember"), TEXT("KickRoomMember call")),
#else
        KickOp,
#endif
        FOnExecuteKickRoomMemberOperationComplete::CreateSPLambda(this, [OnComplete](FError ErrorCode) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            // Check that the operation was successful.
            if (!ErrorCode.WasSuccessful())
            {
                OnComplete.ExecuteIfBound(ErrorCode);
                return;
            }

            // @note: We do not immediately synchronise inside the provider like in CreateRoom. Instead,
            // the room system itself does the synchronisation so that OnRoomMemberLeft event can be raised
            // correctly.

            // Return the success result.
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
        }));
}

void FRoomProvider::SendRoomInvite(
    const FRoomRef &Room,
    const FSendRoomInviteRequest &Request,
    const IRoomSystem::FOnSendRoomInviteComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    // Ensure that this room is for this provider.
    if (!Room->GetRoomId()->GetProviderName().IsEqual(this->GetProviderName()))
    {
        OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::InvalidUser(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Invalid room ID passed to FRoomProvider::SendRoomInvite.")));
        return;
    }
    TSharedRef<FRoomImpl> RoomImpl = StaticCastSharedRef<FRoomImpl>(Room);

    // Create the send room invite operation.
    auto SendOp = this->InitializeOperation(
        this->CreateSendRoomInviteOperation(RoomImpl->GetBoundForLocalUser(), RoomImpl, Request.TargetUser));

    // Execute the send room invite operation.
    this->ExecuteSendRoomInviteOperation(
#if !NO_LOGGING
        this->DumpOperation(SendOp, TEXT("SendRoomInvite"), TEXT("SendRoomInvite call")),
#else
        SendOp,
#endif
        FOnExecuteSendRoomInviteOperationComplete::CreateSPLambda(this, [OnComplete](FError ErrorCode) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            // Check that the operation was successful.
            if (!ErrorCode.WasSuccessful())
            {
                OnComplete.ExecuteIfBound(ErrorCode);
                return;
            }

            // Return the success result.
            OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
        }));
}

FString FRoomProvider::FormatFeatureAttributeKey(const FName &InFeatureName, const FString &InAttributeKey) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return FString::Printf(TEXT("%s%s:%s"), *ReservedRoomAttributePrefix, *InFeatureName.ToString(), *InAttributeKey);
}

TTuple<TSet<FName>, bool> FRoomProvider::ParseFeatureNamesFromRoomAttributes(
    const TMap<FString, FRoomAttribute> &AttributesByKey) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    // Get the Redpoint:EOS:FeatureNames attribute.
    const FRoomAttribute *FeatureAttribute = AttributesByKey.Find(FeatureNamesRoomAttribute);
    if (FeatureAttribute == nullptr)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("%s"),
            *Redpoint::EOS::API::Errors::NotFound(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 FString::Printf(
                     TEXT("Unable to parse feature names from room attributes (missing '%s' attribute)."),
                     *FeatureNamesRoomAttribute))
                 .ToLogString());
        return TTuple<TSet<FName>, bool>(TSet<FName>(), false);
    }

    // Parse into a TSet<FName>.
    TSet<FName> FeatureNames;
    {
        for (const auto &FeatureName : FeatureAttribute->GetStringArray())
        {
            FeatureNames.Add(FName(FeatureName));
        }
    }
    return TTuple<TSet<FName>, bool>(FeatureNames, true);
}

void FRoomProvider::BucketRoomAttributesIntoNonFeatureAndFeatureAttributes(
    const TMap<FString, FRoomAttribute> &AttributesByKey,
    TMap<FString, FRoomAttribute> &NonFeatureAttributes,
    TMap<FName, TMap<FString, FRoomAttribute>> &FeatureAttributes) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    for (const auto &KV : AttributesByKey)
    {
        if (KV.Key.StartsWith(ReservedRoomAttributePrefix, ESearchCase::IgnoreCase))
        {
            auto AttributeKeyWithoutPrefix = KV.Key.Mid(ReservedRoomAttributePrefix.Len());
            int32 NextColon;
            if (AttributeKeyWithoutPrefix.FindChar(TEXT(':'), NextColon))
            {
                auto FeatureName = FName(*AttributeKeyWithoutPrefix.Mid(0, NextColon));
                auto &FeatureAttributeMap = FeatureAttributes.FindOrAdd(FeatureName, TMap<FString, FRoomAttribute>());
                FeatureAttributeMap.Add(AttributeKeyWithoutPrefix.Mid(NextColon + 1), KV.Value);
            }
        }
        else
        {
            NonFeatureAttributes.Add(KV.Key, KV.Value);
        }
    }
}

TOptional<FName> FRoomProvider::GetRoomNamespaceFromAttributesByKey(
    const TMap<FString, FRoomAttribute> &AttributesByKey) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    auto *NamespaceCasingAttr = AttributesByKey.Find(NamespaceCasingRoomAttribute);
    auto *NamespaceFilterAttr = AttributesByKey.Find(NamespaceFilterRoomAttribute);
    if (NamespaceCasingAttr == nullptr || NamespaceFilterAttr == nullptr ||
        NamespaceCasingAttr->Type() != FRoomAttribute::EType::String ||
        NamespaceFilterAttr->Type() != FRoomAttribute::EType::String ||
        !NamespaceCasingAttr->GetString().Equals(NamespaceFilterAttr->GetString(), ESearchCase::IgnoreCase))
    {
        return TOptional<FName>();
    }
    return FName(*NamespaceCasingAttr->GetString());
}

enum ERoomProviderRoomMemberAction
{
    None,
    WaitingForReady,
    ExistingMember,
    AdmitNewMember,
    AlreadyKicking,
    KickIdled,
    KickFeature,
};

void FRoomProvider::SynchroniseRoom(
    const FRoomRef &InRoomAbstract,
    const Synchronisation::FSynchronisationContextRef InSynchronisationContext,
    bool bInitialHostSync)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API;
    using namespace ::Redpoint::EOS::Rooms::Features;
    using namespace ::Redpoint::EOS::Rooms::Synchronisation;
    using namespace ::Redpoint::EOS::Core::Id;

    // Cast the room to the implementation we can modify.
    auto Room = StaticCastSharedRef<FRoomImpl>(InRoomAbstract);

    // Check to see what members have been evicted from the room with this ID.
    auto EvictedMembers = InSynchronisationContext->GetMembersEvictedFromRoom(Room->GetRoomId());
    if (EvictedMembers.Contains(Room->GetBoundForLocalUser()))
    {
        // This user was evicted.
        FRoomMemberSynchronisationDifference::EChangeType Status =
            FRoomMemberSynchronisationDifference::EChangeType::Left;
        auto MemberJoinEvent = InSynchronisationContext->GetFirstSynchronisationEventOfType<
            Redpoint::EOS::API::Lobby::FNotifyLobbyMemberStatusReceived::Result>(
            Room->GetRoomId(),
            TEXT("LobbyMemberStatus"));
        if (MemberJoinEvent.IsValid())
        {
            switch (MemberJoinEvent->GetEventResult().CurrentStatus)
            {
            case EOS_ELobbyMemberStatus::EOS_LMS_LEFT:
                Status = FRoomMemberSynchronisationDifference::EChangeType::Left;
                break;
            case EOS_ELobbyMemberStatus::EOS_LMS_DISCONNECTED:
                Status = FRoomMemberSynchronisationDifference::EChangeType::Disconnected;
                break;
            case EOS_ELobbyMemberStatus::EOS_LMS_KICKED:
                Status = FRoomMemberSynchronisationDifference::EChangeType::Kicked;
                break;
            case EOS_ELobbyMemberStatus::EOS_LMS_CLOSED:
                Status = FRoomMemberSynchronisationDifference::EChangeType::Closed;
                break;
            case EOS_ELobbyMemberStatus::EOS_LMS_JOINED:
            case EOS_ELobbyMemberStatus::EOS_LMS_PROMOTED:
                break;
            default:
                checkf(false, TEXT("Unhandled EOS_ELobbyMemberStatus::EOS_LMS_... value!"));
                break;
            }
        }
        else if (InSynchronisationContext->HasAnticipatedKick(
                     Room->GetRoomId(),
                     Room->GetBoundForLocalUser(),
                     Room->GetBoundForLocalUser()))
        {
            Status = FRoomMemberSynchronisationDifference::EChangeType::Kicked;
            InSynchronisationContext->RemoveAnticipatedKick(
                Room->GetRoomId(),
                Room->GetBoundForLocalUser(),
                Room->GetBoundForLocalUser());
        }
        else
        {
            Status = FRoomMemberSynchronisationDifference::EChangeType::Left;
        }
        InSynchronisationContext->RecordSynchronisationDifference(
            Room,
            MakeShared<FRoomMemberSynchronisationDifference>(Room->GetBoundForLocalUser(), Status, true));

        // Remove any anticipated kicks that are still left.
        InSynchronisationContext->RemoveAnticipatedKicksExcept(
            Room->GetRoomId(),
            Room->GetBoundForLocalUser(),
            TSet<FRoomUserId>());

        // Do not proceed with normal synchronisation.
        return;
    }

    // Create the read operation for this room.
    auto ReadOp = this->InitializeOperation(this->CreateReadRoomOperation(Room->GetBoundForLocalUser(), Room));

    // Compute which features for whom we'll determine presence based on the local room
    // state and not the FeatureNames attribute.
    TSet<FName> FeaturePresenceDeterminedFromLocalState;
    for (const auto &KV : this->FeatureHandlers)
    {
        if (KV.Value->TestFeaturePresenceFromLocalRoomState().IsBound())
        {
            FeaturePresenceDeterminedFromLocalState.Add(KV.Key);
        }
    }

    // Copy all the attributes first so that we can sync features.
    auto AttributesByKey = ReadOp->GetAttributes().Key;

    // Update the room namespace. This isn't an eventable difference, since it should never change.
    auto RoomNamespace = GetRoomNamespaceFromAttributesByKey(AttributesByKey);
    if (RoomNamespace.IsSet())
    {
        if (Room->RoomNamespace.IsNone())
        {
            Room->RoomNamespace = RoomNamespace.GetValue();
        }
        else if (Room->RoomNamespace.IsEqual(RoomNamespace.GetValue()))
        {
            // Expected after the initial synchronisation.
        }
        else
        {
            UE_LOG(
                LogRedpointEOSRooms,
                Error,
                TEXT("Detected that the namespace of room '%s' changed from '%s' to '%s', which is not supported. The "
                     "room namespace attributes must never change on the backend. This change will not be reflected in "
                     "the room system, and may cause unexpected behaviour."),
                *Room->RoomId->ToString(),
                *Room->RoomNamespace.ToString(),
                *RoomNamespace.GetValue().ToString());
        }
    }
    else
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("Unable to read the namespace room '%s' via the room namespace attributes. All rooms must have "
                 "namespace attributes in order to be tracked by the room system. Using this room may cause runtime "
                 "assertions in non-Shipping builds, and unexpected behaviour otherwise."),
            *Room->RoomId->ToString());
    }

    // Parse feature names from attributes.
    auto FeatureNames = this->ParseFeatureNamesFromRoomAttributes(AttributesByKey).Key;

    // Remove any features from the room that are no longer present in the attributes.
    TArray<FName> ExistingFeatureNames;
    Room->RoomFeatures.GenerateKeyArray(ExistingFeatureNames);
    for (const auto &ExistingFeatureName : ExistingFeatureNames)
    {
        if (!FeatureNames.Contains(ExistingFeatureName) &&
            !FeaturePresenceDeterminedFromLocalState.Contains(ExistingFeatureName))
        {
            // Remove the feature - it's no longer used.
            Room->RoomFeatures.Remove(ExistingFeatureName);
            InSynchronisationContext->RecordSynchronisationDifference(
                Room,
                MakeShared<FRoomFeatureSynchronisationDifference>(
                    ExistingFeatureName,
                    FRoomFeatureSynchronisationDifference::EChangeType::Removed));
        }
    }

    // Add any features that are specified by attributes but not currently present.
    TSet<FName> AllFeatureNamesMarkedAsAdded;
    for (const auto &DesiredFeatureName : FeatureNames)
    {
        if (!ExistingFeatureNames.Contains(DesiredFeatureName) &&
            !FeaturePresenceDeterminedFromLocalState.Contains(DesiredFeatureName))
        {
            const auto *FeatureHandler = this->FeatureHandlers.Find(DesiredFeatureName);
            if (FeatureHandler != nullptr)
            {
                auto FeatureInstance =
                    (*FeatureHandler)->ConstructFeatureImplementation().Execute(ReadOp, this->AsShared(), Room);
                if (FeatureInstance != nullptr)
                {
                    Room->RoomFeatures.Add(DesiredFeatureName, FeatureInstance.ToSharedRef());
                    InSynchronisationContext->RecordSynchronisationDifference(
                        Room,
                        MakeShared<FRoomFeatureSynchronisationDifference>(
                            DesiredFeatureName,
                            FRoomFeatureSynchronisationDifference::EChangeType::Added));
                    AllFeatureNamesMarkedAsAdded.Add(DesiredFeatureName);
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("%s"),
                        *Redpoint::EOS::API::Errors::NotFound(
                             ANSI_TO_TCHAR(__FUNCTION__),
                             FString::Printf(
                                 TEXT("The room has a feature '%s' which does not provide an implementation factory on "
                                      "this room provider."),
                                 *DesiredFeatureName.ToString()))
                             .ToLogString());
                }
            }
        }
    }

    // Synchronise feature based on local room state.
    for (const auto &FeatureName : FeaturePresenceDeterminedFromLocalState)
    {
        const auto FeatureHandler = this->FeatureHandlers.FindChecked(FeatureName);
        // @note: No need to check IsBound here since FeatureHandlersFromLocalSettings is already filtered.
        bool FeatureEnabled =
            FeatureHandler->TestFeaturePresenceFromLocalRoomState().Execute(ReadOp, this->AsShared(), Room);
        if (FeatureEnabled)
        {
            if (!Room->RoomFeatures.Contains(FeatureName))
            {
                auto FeatureInstance =
                    FeatureHandler->ConstructFeatureImplementation().Execute(ReadOp, this->AsShared(), Room);
                if (FeatureInstance != nullptr)
                {
                    Room->RoomFeatures.Add(FeatureName, FeatureInstance.ToSharedRef());
                    InSynchronisationContext->RecordSynchronisationDifference(
                        Room,
                        MakeShared<FRoomFeatureSynchronisationDifference>(
                            FeatureName,
                            FRoomFeatureSynchronisationDifference::EChangeType::Added));
                    AllFeatureNamesMarkedAsAdded.Add(FeatureName);
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("%s"),
                        *Redpoint::EOS::API::Errors::NotFound(
                             ANSI_TO_TCHAR(__FUNCTION__),
                             FString::Printf(
                                 TEXT("The room has a feature '%s' which does not provide an implementation factory on "
                                      "this room provider."),
                                 *FeatureName.ToString()))
                             .ToLogString());
                }
            }
        }
        else
        {
            if (Room->RoomFeatures.Contains(FeatureName))
            {
                Room->RoomFeatures.Remove(FeatureName);
                InSynchronisationContext->RecordSynchronisationDifference(
                    Room,
                    MakeShared<FRoomFeatureSynchronisationDifference>(
                        FeatureName,
                        FRoomFeatureSynchronisationDifference::EChangeType::Removed));
            }
        }
    }

    // Run the pre-synchronisation callbacks for the features on this room.
    for (const auto &KV : Room->RoomFeatures)
    {
        // @note: It is not possible for a room to have a feature without a handler
        // previously having constructed it.
        const auto FeatureHandler = this->FeatureHandlers.FindChecked(KV.Key);
        FeatureHandler->UpdateFeatureImplementationDuringPreSynchronisation()
            .Execute(ReadOp, this->AsShared(), Room, KV.Value);

        // @todo: This should be done after PostSync instead.
        //
        // Ensure that the feature request returned by this feature is valid, so that we don't get delayed failures when
        // the user attempts to update the room for the first time.
        /*
        auto FeatureRequest = KV.Value->GetFeatureRequestForUpdate();
        checkf(
            !FeatureRequest.IsValid() || FeatureRequest->IsValid(),
            TEXT("Room feature '%s' returns an invalid non-null feature request after "
                 "UpdateFeatureImplementationDuringPreSynchronisation is called during SynchroniseRoom."),
            *KV.Key.ToString());*/
    }

    // Synchronise basic settings of the room.
    auto RoomInfo = ReadOp->GetInfo().Key;
    if (RoomInfo.IsValid())
    {
        // Synchronise the room owner.
        {
            bool bWasValid = Room->RoomOwner.IsValid();
            if (!Room->RoomOwner.IsValid() ||
                Room->RoomOwner->IsDedicatedServer() != RoomInfo->IsOwnerDedicatedServer() ||
                (Room->RoomOwner->IsDedicatedServer()
                     ? (Room->RoomOwner->GetDedicatedServerClientId() != RoomInfo->GetOwnerDedicatedServerClientId())
                     : (Room->RoomOwner->GetUserId() != RoomInfo->GetOwnerUserId())))
            {
                Room->RoomOwner = RoomInfo->IsOwnerDedicatedServer()
                                      ? MakeShared<FRoomOwner>(RoomInfo->GetOwnerDedicatedServerClientId())
                                      : MakeShared<FRoomOwner>(RoomInfo->GetOwnerUserId());
                if (bWasValid)
                {
                    InSynchronisationContext->RecordSynchronisationDifference(
                        Room,
                        MakeShared<FRoomOwnerSynchronisationDifference>(Room->RoomOwner.ToSharedRef()));
                }
            }
        }

        // Update the maximum number of players.
        {
            uint32 NewMaxMembers = RoomInfo->GetMaxMembers();
            uint32 &RoomMaxMembers = Room->RoomMaxMembers;
            if (RoomMaxMembers != NewMaxMembers)
            {
                bool bWasValid = RoomMaxMembers != UINT32_MAX;
                RoomMaxMembers = NewMaxMembers;
                if (bWasValid)
                {
                    InSynchronisationContext->RecordSynchronisationDifference(
                        Room,
                        MakeShared<FRoomMaxMembersSynchronisationDifference>(RoomMaxMembers));
                }
            }
        }
    }

    // Read all of the room attributes and bucket them into non-feature room attributes
    // and by feature name for the feature post-synchronisation callback.
    TMap<FString, FRoomAttribute> NonFeatureAttributes;
    TMap<FName, TMap<FString, FRoomAttribute>> FeatureAttributes;
    TMap<FName, TMap<FRoomUserId, TMap<FString, FRoomAttribute>>> FeatureMemberAttributes;
    this->BucketRoomAttributesIntoNonFeatureAndFeatureAttributes(
        AttributesByKey,
        NonFeatureAttributes,
        FeatureAttributes);

    // Synchronise the non-feature room attributes.
    for (const auto &Attr : NonFeatureAttributes)
    {
        if (!Room->RoomAttributes.Contains(Attr.Key))
        {
            InSynchronisationContext->RecordSynchronisationDifference(
                Room,
                MakeShared<FRoomAttributeSynchronisationDifference>(
                    Attr.Key,
                    FRoomAttributeSynchronisationDifference::EChangeType::Added,
                    FRoomAttribute(),
                    Attr.Value));
        }
        else if (Room->RoomAttributes[Attr.Key] != Attr.Value)
        {
            InSynchronisationContext->RecordSynchronisationDifference(
                Room,
                MakeShared<FRoomAttributeSynchronisationDifference>(
                    Attr.Key,
                    FRoomAttributeSynchronisationDifference::EChangeType::Modified,
                    Room->RoomAttributes[Attr.Key],
                    Attr.Value));
        }
    }
    for (const auto &Attr : Room->RoomAttributes)
    {
        if (!NonFeatureAttributes.Contains(Attr.Key))
        {
            InSynchronisationContext->RecordSynchronisationDifference(
                Room,
                MakeShared<FRoomAttributeSynchronisationDifference>(
                    Attr.Key,
                    FRoomAttributeSynchronisationDifference::EChangeType::Removed,
                    Attr.Value,
                    FRoomAttribute()));
        }
    }
    Room->RoomAttributes = NonFeatureAttributes;

    // Synchronise the room member user IDs.
    TMap<FRoomUserId, TMap<FString, FRoomAttribute>> LoadedMemberAttributes;
    auto [RoomMembersUnfiltered, RoomMembersError] = ReadOp->GetMembers();
    if (RoomMembersError.WasSuccessful())
    {
        // Map existing and new room members, and filter the incoming list of members by those that have the
        // ReadyRoomMemberAttribute set, so that we don't observe members that haven't set their attributes yet.
        TSet<FRoomUserId> ExistingMembers;
        TSet<FRoomUserId> NewMembers;
        for (const auto &Member : Room->RoomMembers)
        {
            ExistingMembers.Add(Member->GetUserId());
        }
        for (const auto &UserId : RoomMembersUnfiltered)
        {
            auto [RoomMemberAttributes, RoomMemberAttributesError] = ReadOp->GetMemberAttributes(UserId);
            if (RoomMemberAttributesError.WasSuccessful())
            {
                ERoomProviderRoomMemberAction MemberAction = ERoomProviderRoomMemberAction::None;
                if (Room->RoomMembersBeingAutomaticallyKicked.Contains(UserId))
                {
                    // This user is already being automatically kicked. Don't evaluate anything again, nor
                    // should we start another kick operation.
                    MemberAction = ERoomProviderRoomMemberAction::AlreadyKicking;
                }
                else if (ExistingMembers.Contains(UserId))
                {
                    // This user is already a member, so we don't need to check their ready state, but we
                    // do need to add them to 'NewMembers' since this represents the new 'member list', not
                    // just the new members.
                    MemberAction = ERoomProviderRoomMemberAction::ExistingMember;
                }
                else
                {
                    MemberAction = ERoomProviderRoomMemberAction::WaitingForReady;

                    // Is this user indicating that they're ready?
                    auto ReadyRoomMemberAttributeValue = RoomMemberAttributes.Find(ReadyRoomMemberAttribute);
                    FDateTime RoomMemberFirstSeenTimestamp =
                        Room->RoomUnreadyMembersFirstSeenTimestamp.FindOrAdd(UserId, FDateTime::UtcNow());
                    if (ReadyRoomMemberAttributeValue != nullptr &&
                        ReadyRoomMemberAttributeValue->Type() == FRoomAttribute::EType::Bool &&
                        ReadyRoomMemberAttributeValue->GetBool() == true)
                    {
                        // Bucket the member's attributes into non-feature and feature attributes.
                        TMap<FString, FRoomAttribute> NonFeatureMemberAttributes;
                        TMap<FName, TMap<FString, FRoomAttribute>> MemberSpecificFeatureMemberAttributes;
                        this->BucketRoomAttributesIntoNonFeatureAndFeatureAttributes(
                            RoomMemberAttributes,
                            NonFeatureMemberAttributes,
                            MemberSpecificFeatureMemberAttributes);

                        // The user is ready, make sure that all the room features are OK with this room member joining.
                        for (const auto &KV : Room->RoomFeatures)
                        {
                            // @note: It is not possible for a room to have a feature without a handler
                            // previously having constructed it.
                            const auto FeatureHandler = this->FeatureHandlers.FindChecked(KV.Key);
                            if (FeatureHandler->ShouldKickReadyRoomMemberAfterJoin().Execute(
                                    ReadOp,
                                    this->AsShared(),
                                    Room,
                                    KV.Value,
                                    UserId,
                                    MemberSpecificFeatureMemberAttributes.FindOrAdd(
                                        KV.Key,
                                        TMap<FString, FRoomAttribute>())))
                            {
                                MemberAction = ERoomProviderRoomMemberAction::KickFeature;
                                break;
                            }
                        }

                        // If we aren't taking an action to kick the player, then they're ready.
                        if (MemberAction == ERoomProviderRoomMemberAction::WaitingForReady)
                        {
                            MemberAction = ERoomProviderRoomMemberAction::AdmitNewMember;
                        }
                    }
                    // Otherwise, if they haven't become ready within 60 seconds, kick them.
                    else if (
                        RoomMemberFirstSeenTimestamp < FDateTime::UtcNow() - FTimespan::FromSeconds(60) &&
                        Room->RoomOwner.IsValid() &&
                        (Room->RoomOwner->IsDedicatedServer() ||
                         Room->RoomOwner->GetUserId() == Room->GetBoundForLocalUser()))
                    {
                        MemberAction = ERoomProviderRoomMemberAction::KickIdled;
                    }
                }
                checkf(
                    MemberAction != ERoomProviderRoomMemberAction::None,
                    TEXT("Expected RoomMemberAction to not be None!"));

                // Add the user to the new 'member list' if they're an existing member or we're admitting them.
                if (MemberAction == ERoomProviderRoomMemberAction::ExistingMember ||
                    MemberAction == ERoomProviderRoomMemberAction::AdmitNewMember)
                {
                    NewMembers.Add(UserId);
                    LoadedMemberAttributes.Add(UserId, RoomMemberAttributes);
                }
                // Kick the user if a feature requested it or if the user didn't become ready in time.
                else if (
                    MemberAction == ERoomProviderRoomMemberAction::KickFeature ||
                    MemberAction == ERoomProviderRoomMemberAction::KickIdled)
                {
                    const TCHAR *KickReason = MemberAction == ERoomProviderRoomMemberAction::KickFeature
                                                  ? TEXT("room feature requested kick after ready")
                                                  : TEXT("automatic kick due to non-ready member idling");
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("Room member '%s' in room '%s' will be automatically kicked due to reason: '%s'"),
                        *GetUserIdString(UserId),
                        *Room->GetRoomId()->GetRoomId(),
                        KickReason);
                    Room->RoomMembersBeingAutomaticallyKicked.Add(UserId);
                    auto KickOp = this->InitializeOperation(
                        this->CreateKickRoomMemberOperation(Room->GetBoundForLocalUser(), Room, UserId));
                    this->ExecuteKickRoomMemberOperation(
#if !NO_LOGGING
                        this->DumpOperation(KickOp, TEXT("KickRoomMember"), KickReason),
#else
                        KickOp,
#endif
                        FOnExecuteKickRoomMemberOperationComplete::CreateLambda([
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                                                                    KickReason,
#endif
                                                                                    UserId,
                                                                                    Room](FError ErrorCode) {
                            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                            Room->RoomMembersBeingAutomaticallyKicked.Remove(UserId);
                            if (!ErrorCode.WasSuccessful())
                            {
                                UE_LOG(
                                    LogRedpointEOSRooms,
                                    Warning,
                                    TEXT("Unable to kick room member '%s' in room '%s' for reason '%s' automatically: "
                                         "%s"),
                                    *GetUserIdString(UserId),
                                    *Room->GetRoomId()->GetRoomId(),
                                    KickReason,
                                    *ErrorCode.ToLogString());
                            }
                            else
                            {
                                UE_LOG(
                                    LogRedpointEOSRooms,
                                    Verbose,
                                    TEXT("Successfully kicked non-ready room member '%s' in room '%s' for reason '%s' "
                                         "automatically."),
                                    *GetUserIdString(UserId),
                                    *Room->GetRoomId()->GetRoomId(),
                                    KickReason);
                            }
                        }));
                }
            }
        }

        // Generate the synchronisation differences for the members.
        for (const auto &NewMember : NewMembers.Difference(ExistingMembers))
        {
            Room->RoomMembers.Add(MakeShared<FRoomMemberImpl>(NewMember));

            // @note: We don't look at MemberStatus events here, because the only
            // sensible status to report is FRoomMemberSynchronisationDifference::EChangeType::Joined.
            InSynchronisationContext->RecordSynchronisationDifference(
                Room,
                MakeShared<FRoomMemberSynchronisationDifference>(
                    NewMember,
                    FRoomMemberSynchronisationDifference::EChangeType::Joined,
                    Room->GetBoundForLocalUser() == NewMember));
        }
        for (int i = Room->RoomMembers.Num() - 1; i >= 0; i--)
        {
            if (!NewMembers.Contains(Room->RoomMembers[i]->GetUserId()))
            {
                FRoomUserId RemovedUserId = Room->RoomMembers[i]->GetUserId();
                Room->RoomMembers.RemoveAt(i);
                Room->RoomUnreadyMembersFirstSeenTimestamp.Remove(RemovedUserId);
                Room->RoomMembersBeingAutomaticallyKicked.Remove(RemovedUserId);

                // Look at the MemberStatus events to see if we can find a more specific reason as
                // to why this user is leaving the lobby.
                FRoomMemberSynchronisationDifference::EChangeType Status =
                    FRoomMemberSynchronisationDifference::EChangeType::Left;
                auto MemberJoinEvent = InSynchronisationContext->GetFirstSynchronisationEventOfType<
                    Redpoint::EOS::API::Lobby::FNotifyLobbyMemberStatusReceived::Result>(
                    Room->GetRoomId(),
                    TEXT("LobbyMemberStatus"));
                if (MemberJoinEvent.IsValid())
                {
                    switch (MemberJoinEvent->GetEventResult().CurrentStatus)
                    {
                    case EOS_ELobbyMemberStatus::EOS_LMS_LEFT:
                        Status = FRoomMemberSynchronisationDifference::EChangeType::Left;
                        break;
                    case EOS_ELobbyMemberStatus::EOS_LMS_DISCONNECTED:
                        Status = FRoomMemberSynchronisationDifference::EChangeType::Disconnected;
                        break;
                    case EOS_ELobbyMemberStatus::EOS_LMS_KICKED:
                        Status = FRoomMemberSynchronisationDifference::EChangeType::Kicked;
                        break;
                    case EOS_ELobbyMemberStatus::EOS_LMS_CLOSED:
                        Status = FRoomMemberSynchronisationDifference::EChangeType::Closed;
                        break;
                    case EOS_ELobbyMemberStatus::EOS_LMS_JOINED:
                    case EOS_ELobbyMemberStatus::EOS_LMS_PROMOTED:
                        break;
                    default:
                        checkf(false, TEXT("Unhandled EOS_ELobbyMemberStatus::EOS_LMS_... value!"));
                        break;
                    }
                }
                else if (InSynchronisationContext
                             ->HasAnticipatedKick(Room->GetRoomId(), Room->GetBoundForLocalUser(), RemovedUserId))
                {
                    Status = FRoomMemberSynchronisationDifference::EChangeType::Kicked;
                    InSynchronisationContext->RemoveAnticipatedKick(
                        Room->GetRoomId(),
                        Room->GetBoundForLocalUser(),
                        RemovedUserId);
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("Detected that a room member disappeared, but there was no LobbyMemberStatus recorded at "
                             "the time synchronisation ran. This room member will be reported as explicitly leaving "
                             "the room (even though they might have been kicked)."))
                    Status = FRoomMemberSynchronisationDifference::EChangeType::Left;
                }
                InSynchronisationContext->RecordSynchronisationDifference(
                    Room,
                    MakeShared<FRoomMemberSynchronisationDifference>(
                        RemovedUserId,
                        Status,
                        Room->GetBoundForLocalUser() == RemovedUserId));
            }
        }

        // Remove any anticipated kicks for members that are no longer present.
        TSet<FRoomUserId> CurrentRoomMemberIds;
        for (const auto &RoomMember : Room->RoomMembers)
        {
            CurrentRoomMemberIds.Add(RoomMember->GetUserId());
        }
        InSynchronisationContext->RemoveAnticipatedKicksExcept(
            Room->GetRoomId(),
            Room->GetBoundForLocalUser(),
            CurrentRoomMemberIds);
    }

    // Synchronise the room member attributes.
    for (const auto &Member : Room->RoomMembers)
    {
        auto MemberImpl = StaticCastSharedRef<FRoomMemberImpl>(Member);

        auto RoomMemberAttributes = LoadedMemberAttributes.Find(MemberImpl->GetUserId());
        if (RoomMemberAttributes != nullptr)
        {
            // Bucket the member's attributes into non-feature and feature attributes.
            TMap<FString, FRoomAttribute> NonFeatureMemberAttributes;
            TMap<FName, TMap<FString, FRoomAttribute>> MemberSpecificFeatureMemberAttributes;
            this->BucketRoomAttributesIntoNonFeatureAndFeatureAttributes(
                *RoomMemberAttributes,
                NonFeatureMemberAttributes,
                MemberSpecificFeatureMemberAttributes);

            // Synchronise non-feature attributes.
            auto &RoomMemberCurrentAttributes = MemberImpl->GetMutableAttributes();
            for (const auto &Attr : NonFeatureMemberAttributes)
            {
                if (!RoomMemberCurrentAttributes.Contains(Attr.Key))
                {
                    InSynchronisationContext->RecordSynchronisationDifference(
                        Room,
                        MakeShared<FRoomMemberAttributeSynchronisationDifference>(
                            MemberImpl->GetUserId(),
                            Attr.Key,
                            FRoomMemberAttributeSynchronisationDifference::EChangeType::Added,
                            FRoomAttribute(),
                            Attr.Value));
                }
                else if (RoomMemberCurrentAttributes[Attr.Key] != Attr.Value)
                {
                    InSynchronisationContext->RecordSynchronisationDifference(
                        Room,
                        MakeShared<FRoomMemberAttributeSynchronisationDifference>(
                            MemberImpl->GetUserId(),
                            Attr.Key,
                            FRoomMemberAttributeSynchronisationDifference::EChangeType::Modified,
                            RoomMemberCurrentAttributes[Attr.Key],
                            Attr.Value));
                }
            }
            for (const auto &Attr : RoomMemberCurrentAttributes)
            {
                if (!NonFeatureMemberAttributes.Contains(Attr.Key))
                {
                    InSynchronisationContext->RecordSynchronisationDifference(
                        Room,
                        MakeShared<FRoomMemberAttributeSynchronisationDifference>(
                            MemberImpl->GetUserId(),
                            Attr.Key,
                            FRoomMemberAttributeSynchronisationDifference::EChangeType::Removed,
                            Attr.Value,
                            FRoomAttribute()));
                }
            }
            RoomMemberCurrentAttributes = NonFeatureMemberAttributes;

            // Load the member-specific feature attributes into the general feature member attributes map.
            for (const auto &KV : MemberSpecificFeatureMemberAttributes)
            {
                auto &MemberFeatureAttributeMap =
                    FeatureMemberAttributes.FindOrAdd(KV.Key, TMap<FRoomUserId, TMap<FString, FRoomAttribute>>());
                MemberFeatureAttributeMap.Add(MemberImpl->GetUserId(), KV.Value);
            }
        }
    }

#if !NO_LOGGING
    // If we recorded any differences, dump the new room state.
    if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
    {
        auto bRecordedDifferenceCount = InSynchronisationContext->GetSynchronisationDifferences(Room).Num();
        if (bRecordedDifferenceCount > 0)
        {
            ReadOp->InitializeDebugJson();
            this->DumpOperation(
                ReadOp,
                TEXT("SynchroniseRoom"),
                FString::Printf(TEXT("room state has changed with %d differences recorded"), bRecordedDifferenceCount));
        }
    }
#endif

    // Run the post-synchronisation callbacks for the features on this room.
    bool bUpdateRequired = false;
    for (const auto &KV : Room->RoomFeatures)
    {
        // @note: It is not possible for a room to have a feature without a handler
        // previously having constructed it.
        const auto FeatureHandler = this->FeatureHandlers.FindChecked(KV.Key);
        auto FeatureSynchronisationResult =
            FeatureHandler->UpdateFeatureImplementationDuringPostSynchronisation().Execute(
                ReadOp,
                this->AsShared(),
                Room,
                KV.Value,
                FeatureAttributes.FindOrAdd(KV.Key, TMap<FString, FRoomAttribute>()),
                FeatureMemberAttributes.FindOrAdd(KV.Key, TMap<FRoomUserId, TMap<FString, FRoomAttribute>>()));
        if (FeatureSynchronisationResult.RequireRoomUpdate ==
            FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::ERequireRoomUpdate::Yes)
        {
            bUpdateRequired = true;
        }
        if (FeatureSynchronisationResult.StateUpdated ==
                FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::Yes &&
            !AllFeatureNamesMarkedAsAdded.Contains(FeatureHandler->GetFeatureName()))
        {
            InSynchronisationContext->RecordSynchronisationDifference(
                Room,
                MakeShared<FRoomFeatureSynchronisationDifference>(
                    FeatureHandler->GetFeatureName(),
                    FRoomFeatureSynchronisationDifference::EChangeType::Updated));
        }

        // Ensure that the feature request returned by this feature is valid, so that we don't get delayed failures
        // when the user attempts to update the room for the first time.
        auto FeatureRequest = KV.Value->GetFeatureRequestForUpdate();
        checkf(
            !FeatureRequest.IsValid() || FeatureRequest->IsValid(),
            TEXT("Room feature '%s' returns an invalid non-null feature request after "
                 "UpdateFeatureImplementationDuringPostSynchronisation is called during SynchroniseRoom."),
            *KV.Key.ToString());
    }
    if (bUpdateRequired || this->DeferredAutomaticRoomUpdates.Contains(Room))
    {
        if (!this->AutomaticRoomUpdatesInProgress.Contains(Room))
        {
            // Track this room as being updated, which prevents us from starting new updates while the current one is in
            // progress.
            this->AutomaticRoomUpdatesInProgress.Add(Room);
            this->DeferredAutomaticRoomUpdates.Remove(Room);

            // Run UpdateRoomInternal, which will reserialize all of the feature requests and update the room.
            UE_LOG(
                LogRedpointEOSRooms,
                Verbose,
                TEXT("Automatically updating room '%s' because a feature required an update after synchronisation."),
                *Room->GetRoomId()->GetRoomId());
            auto UpdateRoomRequest = FUpdateRoomRequest();
            this->UpdateRoomInternal(
                Room,
                UpdateRoomRequest,
                IRoomSystem::FOnUpdateRoomComplete::CreateSPLambda(
                    this,
                    [this, Room](FError ErrorCode) {
                        if (!ErrorCode.WasSuccessful())
                        {
                            UE_LOG(
                                LogRedpointEOSRooms,
                                Warning,
                                TEXT("Failed to automatically update room '%s' from synchronisation trigger. This room "
                                     "may "
                                     "be automatically updated again to try and bring it into sync. The error was: %s"),
                                *Room->GetRoomId()->GetRoomId(),
                                *ErrorCode.ToLogString());
                        }
                        else
                        {
                            UE_LOG(
                                LogRedpointEOSRooms,
                                Verbose,
                                TEXT("Successfully completed automatic update of room '%s' due to feature "
                                     "synchronisation."),
                                *Room->GetRoomId()->GetRoomId());
                        }

                        // Remove this room from the "being updated" list, which allows automatic synchronisations to
                        // happen again.
                        this->AutomaticRoomUpdatesInProgress.Remove(Room);
                    }),
                TEXT("automatic update due to feature PostSynchronisation callback"));
        }
        else
        {
            // We defer this update until the current automatic update has finished. We need to track this because some
            // features will synchronise their state internally and return RequireRoomUpdate only once for the change
            // that occurred, and we don't want to skip the required update for that just because there is an older one
            // in progress.
            this->DeferredAutomaticRoomUpdates.Add(Room);
        }
    }
}

void FRoomProvider::SynchroniseRooms(
    const TArray<FRoomRef> &InRooms,
    const Synchronisation::FSynchronisationContextRef InSynchronisationContext)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    for (const auto &Room : InRooms)
    {
        this->SynchroniseRoom(Room, InSynchronisationContext, false);
    }
}

void FRoomProvider::Reset()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    this->FeatureHandlers.Reset();
    this->FeatureSetHandlers.Reset();
    this->AutomaticRoomUpdatesInProgress.Reset();
    this->DeferredAutomaticRoomUpdates.Reset();
}

}

#undef ATTEMPT_SET_WITH_DIRECT_ERROR_RETURN
#undef ATTEMPT_SET_WITH_AUTOMATIC_ERROR_RETURN

REDPOINT_EOS_CODE_GUARD_END()