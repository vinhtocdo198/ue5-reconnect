// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProvider.h"

#include "Containers/Ticker.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Lobby/CopyLobbyDetailsHandle.h"
#include "RedpointEOSAPI/Lobby/CopyLobbyDetailsHandleByInviteId.h"
#include "RedpointEOSAPI/Lobby/CopyLobbyDetailsHandleByUiEventId.h"
#include "RedpointEOSAPI/Lobby/CreateLobby.h"
#include "RedpointEOSAPI/Lobby/CreateLobbySearch.h"
#include "RedpointEOSAPI/Lobby/GetRTCRoomName.h"
#include "RedpointEOSAPI/Lobby/KickMember.h"
#include "RedpointEOSAPI/Lobby/LeaveLobby.h"
#include "RedpointEOSAPI/Lobby/NotifyJoinLobbyAccepted.h"
#include "RedpointEOSAPI/Lobby/NotifyLeaveLobbyRequested.h"
#include "RedpointEOSAPI/Lobby/PromoteMember.h"
#include "RedpointEOSAPI/Lobby/QueryInvites.h"
#include "RedpointEOSAPI/Lobby/RejectInvite.h"
#include "RedpointEOSAPI/Lobby/SendInvite.h"
#include "RedpointEOSAPI/Lobby/UpdateLobby.h"
#include "RedpointEOSAPI/Lobby/UpdateLobbyModification.h"
#include "RedpointEOSAPI/LobbyDetails/CopyAttributes.h"
#include "RedpointEOSAPI/LobbyDetails/CopyInfo.h"
#include "RedpointEOSAPI/LobbyModification/SetBucketId.h"
#include "RedpointEOSAPI/LobbyModification/SetInvitesAllowed.h"
#include "RedpointEOSAPI/LobbyModification/SetPermissionLevel.h"
#include "RedpointEOSAPI/LobbySearch/Find.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSAPI/UI/AcknowledgeEventId.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/AliasRoomFeature.h"
#include "RedpointEOSRooms/Features/AliasRoomFeatureImpl.h"
#include "RedpointEOSRooms/Features/AliasRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/AliasRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/AliasRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Features/BucketRoomFeature.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureImpl.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeature.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureImpl.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeature.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureImpl.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeature.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureImpl.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeature.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureImpl.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeature.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureImpl.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeature.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureImpl.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeature.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureImpl.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeature.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureImpl.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeature.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeatureImpl.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeatureSearchStateImpl.h"
#include "RedpointEOSRooms/Logging.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoom.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderCreateOperation.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderJoinOperation.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderLeaveOperation.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderReadOperation.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderSearchOperation.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderTargetMemberOperation.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderUpdateOperation.h"
#include "RedpointEOSRooms/Providers/RoomSearchResultImpl.h"
#include "RedpointEOSRooms/RoomSystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4289788093, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::API::Lobby;

const FName LobbyRoomProviderName = FName(TEXT("Lobby"));

FLobbyRoomProvider::FLobbyRoomProvider(IRoomSystemImpl *InRoomSystemImpl, const FPlatformHandle &InPlatformHandle)
    : FRoomProvider(InPlatformHandle)
    , RoomSystemImpl(InRoomSystemImpl)
    , PlatformHandle(InPlatformHandle)
    , OnRoomJoinRequestedFromSDKEventValue()
    , OnRoomLeaveRequestedFromSDKEventValue()
    , OnRoomInviteLocallyReceivedFromSDKEventValue()
    , OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEventValue()
    , OnRoomInviteLocallyFullyRejectedFromSDKEventValue()
    , OnNativeLobbyUpdateReceivedEventHandle()
    , OnNativeLobbyMemberUpdateReceivedEventHandle()
    , OnNativeLobbyMemberStatusReceivedEventHandle()
    , OnNativeLobbyInviteReceivedEventHandle()
    , OnNativeLobbyInviteAcceptedEventHandle()
    , OnNativeLobbyInviteRejectedEventHandle()
    , OnNativeJoinLobbyAcceptedEventHandle()
    , OnNativeLeaveLobbyRequestedEventHandle()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::LobbyModification;

    // AliasRoomFeature
    this->RegisterFeatureHandler<
            IAliasRoomFeature,
            FAliasRoomFeatureImpl,
            FAliasRoomFeatureRequest,
            IAliasRoomFeatureSearchState>()
        .AllowOnCreate(true)
        .AllowOnJoin(true)
        .ConstructFeatureSearchState(&FAliasRoomFeatureImpl::ConstructFeatureSearchState)
        .ConstructFeatureImplementation(&FAliasRoomFeatureImpl::ConstructFeatureImplementation)
        .UpdateFeatureImplementationDuringPostSynchronisation(
            &FAliasRoomFeatureImpl::UpdateFeatureImplementationDuringPostSynchronisation)
        .SerializeFeatureRequestToRoomMemberAttributes(&FAliasRoomFeatureImpl::SerializeFeatureRequestToRoomAttributes)
        .GetRoomExclusivityKeyFromFeatureRequest(&FAliasRoomFeatureImpl::GetRoomExclusivityKeyFromFeatureRequest)
        .GetRoomExclusivityKeyFromFeature(&FAliasRoomFeatureImpl::GetRoomExclusivityKeyFromFeature);

    // BucketRoomFeature
    this->RegisterFeatureHandler<
            IBucketRoomFeature,
            FBucketRoomFeatureImpl,
            FBucketRoomFeatureRequest,
            IBucketRoomFeatureSearchState>()
        .AllowOnCreate(true)
        .AllowAddOnUpdate(true)
        .AllowModifyOnUpdate(true)
        .AllowDeleteOnUpdate(true)
        .ConstructFeatureSearchState([](const FRoomProviderReadOperationRef &ReadOp,
                                        const TSharedRef<FRoomProvider> &RoomProvider,
                                        const TMap<FString, FRoomAttribute> &FeatureAttributes) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            FString BucketPrefix;
            auto [Attrs, AttrError] = ReadOp->GetAttributes();
            if (AttrError.WasSuccessful())
            {
                auto LobbyRoomProvider = StaticCastSharedRef<FLobbyRoomProvider>(RoomProvider);
                auto RoomNamespace = LobbyRoomProvider->GetRoomNamespaceFromAttributesByKey(Attrs);
                if (RoomNamespace.IsSet())
                {
                    BucketPrefix = RoomNamespace.GetValue().ToString() + TEXT(":");
                }
            }
            auto SearchState = MakeShared<FBucketRoomFeatureSearchStateImpl>();
            auto [Info, InfoError] = ReadOp->GetInfo();
            if (Info.IsValid())
            {
                auto LobbyInfo =
                    StaticCastSharedRef<FLobbyRoomProviderReadOperation::FLobbyRoomInfo>(Info.ToSharedRef());
                SearchState->BucketValue = LobbyInfo->Result.BucketId;
                if (!BucketPrefix.IsEmpty() && SearchState->BucketValue.StartsWith(BucketPrefix))
                {
                    SearchState->BucketValue.MidInline(BucketPrefix.Len());
                }
            }
            return SearchState;
        })
        .ConstructFeatureImplementation([](const FRoomProviderReadOperationRef &ReadOp,
                                           const TSharedRef<FRoomProvider> &RoomProvider,
                                           const FRoomRef &Room) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            return MakeShared<FBucketRoomFeatureImpl>();
        })
        .UpdateFeatureImplementationDuringPostSynchronisation(
            [](const FRoomProviderReadOperationRef &ReadOp,
               const TSharedRef<FRoomProvider> &RoomProvider,
               const FRoomRef &Room,
               const TSharedRef<FBucketRoomFeatureImpl> &Feature,
               const TMap<FString, FRoomAttribute> &FeatureAttributes,
               const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                // @note: See comment in SerializeFeatureRequestToRoomAttributes as to why we no longer use the
                // BucketId field on the lobby info once a user has connected to the room.
                bool bUpdated = false;
                auto BucketAttribute = FeatureAttributes.Find(TEXT("BucketId"));
                if (BucketAttribute != nullptr)
                {
                    bUpdated = bUpdated || (Feature->BucketId != BucketAttribute->GetString());
                    Feature->BucketId = BucketAttribute->GetString();
                }
                return FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult(
                    FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::ERequireRoomUpdate::No,
                    bUpdated ? FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::Yes
                             : FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::No);
            })
        .ApplyFeatureRequestOnCreateRoomOperation([](const FRoomProviderWriteOperationRef &WriteOp,
                                                     const TSharedRef<FBucketRoomFeatureRequest> &FeatureRequest) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto CreateOp = StaticCastSharedRef<FLobbyRoomProviderCreateOperation>(WriteOp);
            auto &Options = CreateOp->GetCreateLobbyOptions();
            Options.BucketId =
                FString::Printf(TEXT("%s:%s"), *CreateOp->GetRoomNamespace().ToString(), *FeatureRequest->Bucket());
            checkf(
                !(*Options.BucketId).IsEmpty(),
                TEXT("Options.BucketId must not be an empty string after ApplyFeatureRequestOnCreateRoomOperation!"));
            return Redpoint::EOS::API::Errors::Success();
        })
        .ApplyFeatureRequestOnUpdateRoomOperation(
            [](const FRoomProviderWriteOperationRef &WriteOp,
               const TSharedPtr<FBucketRoomFeatureRequest> &FeatureRequestOrNullIfDeleted,
               const TSharedPtr<IBucketRoomFeature> &ExistingFeatureOrNullIfNotPresent) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                auto UpdateOp = StaticCastSharedRef<FLobbyRoomProviderUpdateOperation>(WriteOp);
                if (UpdateOp->GetLobbyModification().IsValid())
                {
                    FString Bucket;
                    if (FeatureRequestOrNullIfDeleted.IsValid())
                    {
                        Bucket = FString::Printf(
                            TEXT("%s:%s"),
                            *UpdateOp->GetRoomNamespace().ToString(),
                            *FeatureRequestOrNullIfDeleted->Bucket());
                    }
                    else
                    {
                        Bucket = UpdateOp->GetRoomNamespace().ToString();
                    }
                    auto Result = FSetBucketId::Execute(
                        UpdateOp->GetLobbyModification().ToSharedRef(),
                        FSetBucketId::Options{Bucket});
#if !NO_LOGGING
                    if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
                    {
                        auto OperationObject = UpdateOp->AddUpdateToDebugJson("SetBucketId", Result);
                        OperationObject->SetStringField("BucketId", Bucket);
                    }
#endif
                    return Redpoint::EOS::API::ConvertError(Result);
                }
                return Redpoint::EOS::API::Errors::Success();
            })
        .SerializeFeatureRequestToRoomAttributes([](const FRoomProviderWriteOperationRef &WriteOp,
                                                    const TSharedRef<FBucketRoomFeatureRequest> &FeatureRequest) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            // @hack: This is a workaround for an issue where calling SetBucketId while there are other members
            // connected causes those members to see the bucket ID to change to an empty/null string instead. By
            // serializing the bucket ID as an attribute, we ensure that clients always see the correct value regardless
            // of bugs in the EOS SDK:
            //
            // https://eoshelp.epicgames.com/s/case/500QP000006FkdNYAS/calling-eoslobbymodificationsetbucketid-with-other-members-causes-those-members-to-see-bucketid-as-an-empty-value-when-they-call-eoslobbydetailscopyinfo?language=en_US
            // (private case)
            return TMap<FString, FRoomAttribute>{
                {TEXT("BucketId"), FeatureRequest->Bucket()},
            };
        });

    // InvitesRoomFeature
    this->RegisterFeatureHandler<
            IInvitesRoomFeature,
            FInvitesRoomFeatureImpl,
            FInvitesRoomFeatureRequest,
            IInvitesRoomFeatureSearchState>()
        .AllowOnCreate(true)
        .AllowAddOnUpdate(true)
        .AllowModifyOnUpdate(true)
        .AllowDeleteOnUpdate(true)
        .ConstructFeatureSearchState([](const FRoomProviderReadOperationRef &ReadOp,
                                        const TSharedRef<FRoomProvider> &RoomProvider,
                                        const TMap<FString, FRoomAttribute> &FeatureAttributes) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto SearchState = MakeShared<FInvitesRoomFeatureSearchStateImpl>();
            auto [Info, InfoError] = ReadOp->GetInfo();
            if (Info.IsValid())
            {
                auto LobbyInfo =
                    StaticCastSharedRef<FLobbyRoomProviderReadOperation::FLobbyRoomInfo>(Info.ToSharedRef());
                SearchState->bRejoinAfterKickRequiresInvite = LobbyInfo->Result.bRejoinAfterKickRequiresInvite;
            }
            return SearchState;
        })
        .TestFeaturePresenceFromSearchReadOp([](const FRoomProviderReadOperationRef &ReadOp,
                                                const TSharedRef<FRoomProvider> &RoomProvider,
                                                const TMap<FString, FRoomAttribute> &FeatureAttributes) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto [Info, InfoError] = ReadOp->GetInfo();
            if (Info.IsValid())
            {
                auto LobbyInfo =
                    StaticCastSharedRef<FLobbyRoomProviderReadOperation::FLobbyRoomInfo>(Info.ToSharedRef());
                return LobbyInfo->Result.bAllowInvites;
            }
            return false;
        })
        .ConstructFeatureImplementation([](const FRoomProviderReadOperationRef &ReadOp,
                                           const TSharedRef<FRoomProvider> &RoomProvider,
                                           const FRoomRef &Room) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            return MakeShared<FInvitesRoomFeatureImpl>();
        })
        .TestFeaturePresenceFromLocalRoomState([](const FRoomProviderReadOperationRef &ReadOp,
                                                  const TSharedRef<FRoomProvider> &RoomProvider,
                                                  const FRoomRef &Room) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto [Info, InfoError] = ReadOp->GetInfo();
            if (Info.IsValid())
            {
                auto LobbyInfo =
                    StaticCastSharedRef<FLobbyRoomProviderReadOperation::FLobbyRoomInfo>(Info.ToSharedRef());
                return LobbyInfo->Result.bAllowInvites;
            }
            return false;
        })
        .UpdateFeatureImplementationDuringPreSynchronisation([](const FRoomProviderReadOperationRef &ReadOp,
                                                                const TSharedRef<FRoomProvider> &RoomProvider,
                                                                const FRoomRef &Room,
                                                                const TSharedRef<FInvitesRoomFeatureImpl> &Feature) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto [Info, InfoError] = ReadOp->GetInfo();
            if (Info.IsValid())
            {
                auto LobbyInfo =
                    StaticCastSharedRef<FLobbyRoomProviderReadOperation::FLobbyRoomInfo>(Info.ToSharedRef());
                Feature->bRejoinAfterKickRequiresInviteValue = LobbyInfo->Result.bRejoinAfterKickRequiresInvite;
            }
        })
        .ApplyFeatureRequestOnCreateRoomOperation([](const FRoomProviderWriteOperationRef &WriteOp,
                                                     const TSharedRef<FInvitesRoomFeatureRequest> &FeatureRequest) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto CreateOp = StaticCastSharedRef<FLobbyRoomProviderCreateOperation>(WriteOp);
            auto &Options = CreateOp->GetCreateLobbyOptions();
            Options.bRejoinAfterKickRequiresInvite = FeatureRequest->RejoinAfterKickRequiresInvite();
            return Redpoint::EOS::API::Errors::Success();
        })
        .ApplyFeatureRequestOnPostCreateRoomOperation([](const FRoomProviderWriteOperationRef &WriteOp,
                                                         const TSharedRef<FInvitesRoomFeatureRequest> &FeatureRequest) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto UpdateOp = StaticCastSharedRef<FLobbyRoomProviderUpdateOperation>(WriteOp);
            if (UpdateOp->GetLobbyModification().IsValid())
            {
                auto Result = FSetInvitesAllowed::Execute(
                    UpdateOp->GetLobbyModification().ToSharedRef(),
                    FSetInvitesAllowed::Options{true});
#if !NO_LOGGING
                if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
                {
                    auto OperationObject = UpdateOp->AddUpdateToDebugJson("SetInvitesAllowed", Result);
                    OperationObject->SetBoolField("Allowed", true);
                }
#endif
                return Redpoint::EOS::API::ConvertError(Result);
            }
            return Redpoint::EOS::API::Errors::Success();
        })
        .ApplyFeatureRequestOnUpdateRoomOperation([](const FRoomProviderWriteOperationRef &WriteOp,
                                                     const TSharedPtr<FInvitesRoomFeatureRequest>
                                                         &FeatureRequestOrNullIfDeleted,
                                                     const TSharedPtr<FInvitesRoomFeatureImpl>
                                                         &ExistingFeatureOrNullIfNotPresent) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto UpdateOp = StaticCastSharedRef<FLobbyRoomProviderUpdateOperation>(WriteOp);
            if (UpdateOp->GetLobbyModification().IsValid())
            {
                if (FeatureRequestOrNullIfDeleted.IsValid())
                {
                    // Read the existing value of bRejoinAfterKickRequiresInvite directly off the
                    // EOS SDK to compare with what was requested. We can't read off FInvitesRoomFeatureImpl
                    // in case it doesn't exist (i.e. invites are being turned on), and we can't
                    // assume it is false because invites may have been turned on previously when
                    // the room was created.
                    bool bRejoinAfterKickRequiresInvite = false;
                    auto [Info, InfoError] = UpdateOp->GetInfo();
                    if (Info.IsValid())
                    {
                        auto LobbyInfo =
                            StaticCastSharedRef<FLobbyRoomProviderReadOperation::FLobbyRoomInfo>(Info.ToSharedRef());
                        bRejoinAfterKickRequiresInvite = LobbyInfo->Result.bRejoinAfterKickRequiresInvite;
                    }

                    if (bRejoinAfterKickRequiresInvite !=
                        FeatureRequestOrNullIfDeleted->RejoinAfterKickRequiresInvite())
                    {
                        return Redpoint::EOS::API::Errors::InvalidParameters(
                            ANSI_TO_TCHAR(__FUNCTION__),
                            TEXT("The value of RejoinAfterKickRequiresInvite can not be changed after room creation."));
                    }
                }

                auto Result = FSetInvitesAllowed::Execute(
                    UpdateOp->GetLobbyModification().ToSharedRef(),
                    FSetInvitesAllowed::Options{FeatureRequestOrNullIfDeleted.IsValid()});
#if !NO_LOGGING
                if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
                {
                    auto OperationObject = UpdateOp->AddUpdateToDebugJson("SetInvitesAllowed", Result);
                    OperationObject->SetBoolField("Allowed", FeatureRequestOrNullIfDeleted.IsValid());
                }
#endif
                return Redpoint::EOS::API::ConvertError(Result);
            }
            return Redpoint::EOS::API::Errors::Success();
        });

    // JoinByIdRoomFeature
    this->RegisterFeatureHandler<
            IJoinByIdRoomFeature,
            FJoinByIdRoomFeatureImpl,
            FJoinByIdRoomFeatureRequest,
            IJoinByIdRoomFeatureSearchState>()
        .AllowOnCreate(true)
        .ConstructFeatureSearchState([](const FRoomProviderReadOperationRef &ReadOp,
                                        const TSharedRef<FRoomProvider> &RoomProvider,
                                        const TMap<FString, FRoomAttribute> &FeatureAttributes) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            return MakeShared<FJoinByIdRoomFeatureSearchStateImpl>();
        })
        .ConstructFeatureImplementation([](const FRoomProviderReadOperationRef &ReadOp,
                                           const TSharedRef<FRoomProvider> &RoomProvider,
                                           const FRoomRef &Room) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            return MakeShared<FJoinByIdRoomFeatureImpl>();
        })
        .ApplyFeatureRequestOnCreateRoomOperation([](const FRoomProviderWriteOperationRef &WriteOp,
                                                     const TSharedRef<FJoinByIdRoomFeatureRequest> &FeatureRequest) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto CreateOp = StaticCastSharedRef<FLobbyRoomProviderCreateOperation>(WriteOp);
            auto &Options = CreateOp->GetCreateLobbyOptions();
            Options.bEnableJoinById = true;
            return Redpoint::EOS::API::Errors::Success();
        });

    // OwnerMigrationRoomFeature
    this->RegisterFeatureHandler<
            IOwnerMigrationRoomFeature,
            FOwnerMigrationRoomFeatureImpl,
            FOwnerMigrationRoomFeatureRequest,
            IOwnerMigrationRoomFeatureSearchState>()
        .AllowOnCreate(true)
        .ConstructFeatureSearchState([](const FRoomProviderReadOperationRef &ReadOp,
                                        const TSharedRef<FRoomProvider> &RoomProvider,
                                        const TMap<FString, FRoomAttribute> &FeatureAttributes) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            return MakeShared<FOwnerMigrationRoomFeatureSearchStateImpl>();
        })
        .ConstructFeatureImplementation([](const FRoomProviderReadOperationRef &ReadOp,
                                           const TSharedRef<FRoomProvider> &RoomProvider,
                                           const FRoomRef &Room) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            return MakeShared<FOwnerMigrationRoomFeatureImpl>();
        })
        .ApplyFeatureRequestOnCreateRoomOperation(
            [](const FRoomProviderWriteOperationRef &WriteOp,
               const TSharedRef<FOwnerMigrationRoomFeatureRequest> &FeatureRequest) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                auto CreateOp = StaticCastSharedRef<FLobbyRoomProviderCreateOperation>(WriteOp);
                auto &Options = CreateOp->GetCreateLobbyOptions();
                Options.bDisableHostMigration = false;
                return Redpoint::EOS::API::Errors::Success();
            })
        .ApplyOmittedFeatureRequestOnCreateRoomOperation([](const FRoomProviderWriteOperationRef &WriteOp) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto CreateOp = StaticCastSharedRef<FLobbyRoomProviderCreateOperation>(WriteOp);
            auto &Options = CreateOp->GetCreateLobbyOptions();
            Options.bDisableHostMigration = true;
            return Redpoint::EOS::API::Errors::Success();
        });

    // PartyConfigurationRoomFeature
    this->RegisterFeatureHandler<
            IPartyConfigurationRoomFeature,
            FPartyConfigurationRoomFeatureImpl,
            FPartyConfigurationRoomFeatureRequest,
            IPartyConfigurationRoomFeatureSearchState>()
        .AllowOnCreate(true)
        .AllowOnJoin(true) // @note: Required for providing the password when joining lobbies.
        .AllowAddOnUpdate(true)
        .AllowModifyOnUpdate(true)
        .AllowDeleteOnUpdate(true)
        .ConstructFeatureSearchState(&FPartyConfigurationRoomFeatureImpl::ConstructFeatureSearchState)
        .ConstructFeatureImplementation(&FPartyConfigurationRoomFeatureImpl::ConstructFeatureImplementation)
        .UpdateFeatureImplementationDuringPostSynchronisation(
            &FPartyConfigurationRoomFeatureImpl::UpdateFeatureImplementationDuringPostSynchronisation)
        .SerializeFeatureRequestToRoomAttributes(
            &FPartyConfigurationRoomFeatureImpl::SerializeFeatureRequestToRoomAttributes)
        .SerializeFeatureRequestToRoomMemberAttributes(
            &FPartyConfigurationRoomFeatureImpl::SerializeFeatureRequestToRoomMemberAttributes)
        .ShouldKickReadyRoomMemberAfterJoin(&FPartyConfigurationRoomFeatureImpl::ShouldKickReadyRoomMemberAfterJoin);

    // PartyTypeRoomFeature
    this->RegisterFeatureHandler<
            IPartyTypeRoomFeature,
            FPartyTypeRoomFeatureImpl,
            FPartyTypeRoomFeatureRequest,
            IPartyTypeRoomFeatureSearchState>()
        .AllowOnCreate(true)
        .ConstructFeatureSearchState(&FPartyTypeRoomFeatureImpl::ConstructFeatureSearchState)
        .ConstructFeatureImplementation(&FPartyTypeRoomFeatureImpl::ConstructFeatureImplementation)
        .UpdateFeatureImplementationDuringPostSynchronisation(
            &FPartyTypeRoomFeatureImpl::UpdateFeatureImplementationDuringPostSynchronisation)
        .SerializeFeatureRequestToRoomAttributes(&FPartyTypeRoomFeatureImpl::SerializeFeatureRequestToRoomAttributes)
        .GetRoomExclusivityKeyFromFeatureRequest(&FPartyTypeRoomFeatureImpl::GetRoomExclusivityKeyFromFeatureRequest)
        .GetRoomExclusivityKeyFromFeature(&FPartyTypeRoomFeatureImpl::GetRoomExclusivityKeyFromFeature)
        .GetRoomExclusivityKeyFromFeatureSearchState(
            &FPartyTypeRoomFeatureImpl::GetRoomExclusivityKeyFromFeatureSearchState);

    // PresenceRoomFeature
    this->RegisterFeatureHandler<
            IPresenceRoomFeature,
            FPresenceRoomFeatureImpl,
            FPresenceRoomFeatureRequest,
            IPresenceRoomFeatureSearchState>()
        .AllowOnCreate(true)
        .AllowModifyOnUpdate(true)
        .AllowOnJoin(true)
        .ConstructFeatureSearchState([](const FRoomProviderReadOperationRef &ReadOp,
                                        const TSharedRef<FRoomProvider> &RoomProvider,
                                        const TMap<FString, FRoomAttribute> &FeatureAttributes) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto SearchState = MakeShared<FPresenceRoomFeatureSearchStateImpl>();
            auto [Info, InfoError] = ReadOp->GetInfo();
            if (Info.IsValid())
            {
                auto LobbyInfo =
                    StaticCastSharedRef<FLobbyRoomProviderReadOperation::FLobbyRoomInfo>(Info.ToSharedRef());
                SearchState->bAllowJoinFromPresence =
                    LobbyInfo->Result.PermissionLevel == EOS_ELobbyPermissionLevel::EOS_LPL_JOINVIAPRESENCE ||
                    LobbyInfo->Result.PermissionLevel == EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
            }
            return SearchState;
        })
        .ConstructFeatureImplementation([](const FRoomProviderReadOperationRef &ReadOp,
                                           const TSharedRef<FRoomProvider> &RoomProvider,
                                           const FRoomRef &Room) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            return MakeShared<FPresenceRoomFeatureImpl>();
        })
        .TestFeaturePresenceFromLocalRoomState([](const FRoomProviderReadOperationRef &ReadOp,
                                                  const TSharedRef<FRoomProvider> &RoomProvider,
                                                  const FRoomRef &Room) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto [Info, InfoError] = ReadOp->GetInfo();
            if (Info.IsValid())
            {
                auto LobbyInfo =
                    StaticCastSharedRef<FLobbyRoomProviderReadOperation::FLobbyRoomInfo>(Info.ToSharedRef());
                return LobbyInfo->Result.bPresenceEnabled;
            }
            return false;
        })
        .UpdateFeatureImplementationDuringPreSynchronisation([](const FRoomProviderReadOperationRef &ReadOp,
                                                                const TSharedRef<FRoomProvider> &RoomProvider,
                                                                const FRoomRef &Room,
                                                                const TSharedRef<FPresenceRoomFeatureImpl> &Feature) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto [Info, InfoError] = ReadOp->GetInfo();
            if (Info.IsValid())
            {
                auto LobbyInfo =
                    StaticCastSharedRef<FLobbyRoomProviderReadOperation::FLobbyRoomInfo>(Info.ToSharedRef());
                Feature->bAllowJoinFromPresence =
                    LobbyInfo->Result.PermissionLevel == EOS_ELobbyPermissionLevel::EOS_LPL_JOINVIAPRESENCE ||
                    LobbyInfo->Result.PermissionLevel == EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
            }
        })
        .ApplyFeatureRequestOnCreateRoomOperation([](const FRoomProviderWriteOperationRef &WriteOp,
                                                     const TSharedRef<FPresenceRoomFeatureRequest> &FeatureRequest) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto CreateOp = StaticCastSharedRef<FLobbyRoomProviderCreateOperation>(WriteOp);
            auto &Options = CreateOp->GetCreateLobbyOptions();
            Options.bPresenceEnabled = true;
            return Redpoint::EOS::API::Errors::Success();
        })
        .ApplyFeatureRequestOnJoinRoomOperation([](const FRoomProviderWriteOperationRef WriteOp,
                                                   const TSharedRef<FPresenceRoomFeatureRequest> &FeatureRequest) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto JoinOp = StaticCastSharedRef<FLobbyRoomProviderJoinOperation>(WriteOp);
            auto &Options = JoinOp->GetJoinLobbyByIdOptions();
            Options.bPresenceEnabled = true;
            return Redpoint::EOS::API::Errors::Success();
        })
        .GetRoomExclusivityKeyFromFeatureRequest(&FPresenceRoomFeatureImpl::GetRoomExclusivityKeyFromFeatureRequest)
        .GetRoomExclusivityKeyFromFeature(&FPresenceRoomFeatureImpl::GetRoomExclusivityKeyFromFeature);

    // PublicAdvertisementRoomFeature
    this->RegisterFeatureHandler<
            IPublicAdvertisementRoomFeature,
            FPublicAdvertisementRoomFeatureImpl,
            FPublicAdvertisementRoomFeatureRequest,
            IPublicAdvertisementRoomFeatureSearchState>()
        .AllowOnCreate(true)
        .AllowAddOnUpdate(true)
        .AllowModifyOnUpdate(true)
        .AllowDeleteOnUpdate(true)
        .ConstructFeatureSearchState([](const FRoomProviderReadOperationRef &ReadOp,
                                        const TSharedRef<FRoomProvider> &RoomProvider,
                                        const TMap<FString, FRoomAttribute> &FeatureAttributes) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            return MakeShared<FPublicAdvertisementRoomFeatureSearchStateImpl>();
        })
        .ConstructFeatureImplementation([](const FRoomProviderReadOperationRef &ReadOp,
                                           const TSharedRef<FRoomProvider> &RoomProvider,
                                           const FRoomRef &Room) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            return MakeShared<FPublicAdvertisementRoomFeatureImpl>();
        })
        .TestFeaturePresenceFromLocalRoomState([](const FRoomProviderReadOperationRef &ReadOp,
                                                  const TSharedRef<FRoomProvider> &RoomProvider,
                                                  const FRoomRef &Room) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto [Info, InfoError] = ReadOp->GetInfo();
            if (Info.IsValid())
            {
                auto LobbyInfo =
                    StaticCastSharedRef<FLobbyRoomProviderReadOperation::FLobbyRoomInfo>(Info.ToSharedRef());
                return LobbyInfo->Result.PermissionLevel == EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
            }
            return false;
        });

    // PublicMemberListRoomFeature
    this->RegisterFeatureHandler<
            IPublicMemberListRoomFeature,
            FPublicMemberListRoomFeatureImpl,
            FPublicMemberListRoomFeatureRequest,
            IPublicMemberListRoomFeatureSearchState>()
        .AllowOnCreate(true)
        .AllowAddOnUpdate(true)
        .AllowModifyOnUpdate(true)
        .AllowDeleteOnUpdate(true)
        .ConstructFeatureSearchState([](const FRoomProviderReadOperationRef &ReadOp,
                                        const TSharedRef<FRoomProvider> &RoomProvider,
                                        const TMap<FString, FRoomAttribute> &FeatureAttributes) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            using namespace ::Redpoint::EOS::Core::Id;
            auto SearchState = MakeShared<FPublicMemberListRoomFeatureSearchStateImpl>();
            auto MaxMembersAttribute = FeatureAttributes.Find(TEXT("MaxMembers"));
            auto CurrentMembersAttribute = FeatureAttributes.Find(TEXT("CurrentMembers"));
            if (MaxMembersAttribute != nullptr && MaxMembersAttribute->Type() == FRoomAttribute::EType::Int64)
            {
                SearchState->MaxMembersValue = (uint32)MaxMembersAttribute->GetInt64();
            }
            if (CurrentMembersAttribute != nullptr && CurrentMembersAttribute->Type() == FRoomAttribute::EType::String)
            {
                auto CurrentMembersStr = CurrentMembersAttribute->GetStringArray();
                SearchState->CurrentMembersValue = TArray<FRoomUserId>();
                for (const auto &Str : CurrentMembersStr)
                {
                    auto ParsedMemberUserId = TryParseAccountId(Str);
                    if (ParsedMemberUserId.IsSet())
                    {
                        SearchState->CurrentMembersValue.Add(ParsedMemberUserId.GetValue());
                    }
                    else
                    {
                        UE_LOG(
                            LogRedpointEOSRooms,
                            Warning,
                            TEXT("Ignoring member user ID '%s' in search result as it can not be parsed as a product "
                                 "user ID."),
                            *Str);
                    }
                }
            }
            return SearchState;
        })
        .ConstructFeatureImplementation([](const FRoomProviderReadOperationRef &ReadOp,
                                           const TSharedRef<FRoomProvider> &RoomProvider,
                                           const FRoomRef &Room) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            TArray<FRoomUserId> MemberUserIds;
            for (const auto &Member : Room->GetMembers())
            {
                MemberUserIds.Add(Member->GetUserId());
            }
            return MakeShared<FPublicMemberListRoomFeatureImpl>(MemberUserIds, Room->GetRoomMaxMembers());
        })
        .UpdateFeatureImplementationDuringPostSynchronisation(
            [](const FRoomProviderReadOperationRef &ReadOp,
               const TSharedRef<FRoomProvider> &RoomProvider,
               const FRoomRef &Room,
               const TSharedRef<FPublicMemberListRoomFeatureImpl> &Feature,
               const TMap<FString, FRoomAttribute> &FeatureAttributes,
               const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                // We compare the current room members with the snapshot taken inside the feature to figure out whether
                // or not we need to update the room attributes with a new list of members.
                TSet<FRoomUserId> ExpectedCurrentMembers;
                for (const auto &Member : Room->GetMembers())
                {
                    ExpectedCurrentMembers.Add(Member->GetUserId());
                }
                TSet<FRoomUserId> FeatureCurrentMembers;
                for (const auto &MemberId : Feature->CurrentMembersValue)
                {
                    FeatureCurrentMembers.Add(MemberId);
                }
                bool bRequiresUpdate = FeatureCurrentMembers.Difference(ExpectedCurrentMembers).Num() != 0 ||
                                       ExpectedCurrentMembers.Difference(FeatureCurrentMembers).Num() != 0 ||
                                       Room->GetRoomMaxMembers() != Feature->MaxMembersValue;
                TSharedRef<FRoomImpl> RoomImpl = StaticCastSharedRef<FRoomImpl>(Room);
                // @todo: bAuthoritative should be a parameter passed into all
                // UpdateFeatureImplementationDuringPostSynchronisation callbacks, since some features will want to
                // update member attributes, and others can only run on the host.
                auto bAuthoritative = RoomImpl->GetBoundForLocalUser() == RoomImpl->GetRoomOwner()->GetUserId();
                if (bRequiresUpdate && bAuthoritative)
                {
                    bool bUpdated = false;
                    TArray<FRoomUserId> ArrayCurrentMembers;
                    for (const auto &MemberId : ExpectedCurrentMembers)
                    {
                        ArrayCurrentMembers.Add(MemberId);
                    }
                    if (Feature->CurrentMembersValue != ArrayCurrentMembers)
                    {
                        bUpdated = true;
                        Feature->CurrentMembersValue = ArrayCurrentMembers;
                    }
                    if (Feature->MaxMembersValue != Room->GetRoomMaxMembers())
                    {
                        bUpdated = true;
                        Feature->MaxMembersValue = Room->GetRoomMaxMembers();
                    }
                    return FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult(
                        FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::ERequireRoomUpdate::Yes,
                        bUpdated
                            ? FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::Yes
                            : FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::No);
                }
                else
                {
                    return FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult(
                        FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::ERequireRoomUpdate::No,
                        FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::No);
                }
            })
        .SerializeFeatureRequestToRoomAttributes(
            [](const FRoomProviderWriteOperationRef &WriteOp,
               const TSharedRef<FPublicMemberListRoomFeatureRequest> &FeatureRequest) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                using namespace ::Redpoint::EOS::Core::Id;
                auto [Info, InfoError] = WriteOp->GetInfo();
                auto [Members, MembersError] = WriteOp->GetMembers();
                if (!InfoError.WasSuccessful() || !MembersError.WasSuccessful() || !Info.IsValid())
                {
                    return TMap<FString, FRoomAttribute>();
                }
                TArray<FString> MemberListSerialized;
                for (const auto &MemberId : Members)
                {
                    MemberListSerialized.Add(GetUserIdString(MemberId));
                }
                return TMap<FString, FRoomAttribute>{
                    {TEXT("MaxMembers"), (int64)Info->GetMaxMembers()},
                    {TEXT("CurrentMembers"), MemberListSerialized},
                };
            });

    // VoiceChatRoomFeature
    this->RegisterFeatureHandler<
            IVoiceChatRoomFeature,
            FVoiceChatRoomFeatureImpl,
            FVoiceChatRoomFeatureRequest,
            IVoiceChatRoomFeatureSearchState>()
        .AllowOnCreate(true)
        .ConstructFeatureSearchState([](const FRoomProviderReadOperationRef &ReadOp,
                                        const TSharedRef<FRoomProvider> &RoomProvider,
                                        const TMap<FString, FRoomAttribute> &FeatureAttributes) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            return MakeShared<FVoiceChatRoomFeatureSearchStateImpl>();
        })
        .ConstructFeatureImplementation([](const FRoomProviderReadOperationRef &ReadOp,
                                           const TSharedRef<FRoomProvider> &RoomProvider,
                                           const FRoomRef &Room) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            return MakeShared<FVoiceChatRoomFeatureImpl>(
                FVoiceChatRoomFeatureImpl::FGetRTCRoomNameDelegate::CreateSPLambda(
                    Room,
                    [Room, RoomProvider, ReadOp]() {
                        using namespace ::Redpoint::EOS::Core::Id;
                        auto LobbyRoomProvider = StaticCastSharedRef<FLobbyRoomProvider>(RoomProvider);
                        EOS_EResult ResultCode;
                        FString RTCRoomName;
                        Redpoint::EOS::API::Lobby::FGetRTCRoomName::Execute(
                            LobbyRoomProvider->PlatformHandle,
                            FGetRTCRoomName::Options{
                                GetProductUserId(ReadOp->GetActorUserId()),
                                Room->GetRoomId()->GetRoomId()},
                            ResultCode,
                            RTCRoomName);
                        if (ResultCode != EOS_EResult::EOS_Success)
                        {
                            UE_LOG(
                                LogRedpointEOSRooms,
                                Error,
                                TEXT("%s"),
                                *ConvertError(
                                     ANSI_TO_TCHAR(__FUNCTION__),
                                     TEXT("Unable to retrieve RTC room name for lobby."),
                                     ResultCode)
                                     .ToLogString());
                        }
                        return RTCRoomName;
                    }));
        })
        .SerializeFeatureRequestToRoomMemberAttributes(
            [](const FRoomProviderWriteOperationRef &WriteOp,
               const TSharedRef<FVoiceChatRoomFeatureRequest> &FeatureRequest) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                return TMap<FString, FRoomAttribute>{
                    {TEXT("JoinFlags"), (int64)FeatureRequest->JoinFlags()},
                    {TEXT("UseManualAudioInput"), FeatureRequest->UseManualAudioInput()},
                    {TEXT("UseManualAudioOutput"), FeatureRequest->UseManualAudioOutput()},
                    {TEXT("LocalAudioDeviceInputStartsMuted"), FeatureRequest->LocalAudioDeviceInputStartsMuted()},
                };
            })
        .UpdateFeatureImplementationDuringPostSynchronisation(
            [](const FRoomProviderReadOperationRef &ReadOp,
               const TSharedRef<FRoomProvider> &RoomProvider,
               const FRoomRef &Room,
               const TSharedRef<FVoiceChatRoomFeatureImpl> &Feature,
               const TMap<FString, FRoomAttribute> &FeatureAttributes,
               const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                auto [Info, InfoError] = ReadOp->GetInfo();
                bool bUpdated = false;
                if (Info.IsValid())
                {
                    auto LobbyInfo =
                        StaticCastSharedRef<FLobbyRoomProviderReadOperation::FLobbyRoomInfo>(Info.ToSharedRef());
                    auto LobbyRoom = StaticCastSharedRef<FLobbyRoom>(Room);
                    auto ThisUserAttributes = FeatureMemberAttributes.Find(LobbyRoom->GetBoundForLocalUser());
                    if (ThisUserAttributes != nullptr)
                    {
                        auto JoinFlagsAttribute = ThisUserAttributes->Find(TEXT("JoinFlags"));
                        auto UseManualAudioInputAttribute = ThisUserAttributes->Find(TEXT("UseManualAudioInput"));
                        auto UseManualAudioOutputAttribute = ThisUserAttributes->Find(TEXT("UseManualAudioOutput"));
                        auto LocalAudioDeviceInputStartsMutedAttribute =
                            ThisUserAttributes->Find(TEXT("LocalAudioDeviceInputStartsMuted"));
                        if (JoinFlagsAttribute != nullptr)
                        {
                            bUpdated = bUpdated || (Feature->JoinFlagsValue !=
                                                    (RTC::EJoinRoomOptions::Type)JoinFlagsAttribute->GetInt64());
                            Feature->JoinFlagsValue = (RTC::EJoinRoomOptions::Type)JoinFlagsAttribute->GetInt64();
                        }
                        if (UseManualAudioInputAttribute != nullptr)
                        {
                            bUpdated = bUpdated ||
                                       (Feature->bUseManualAudioInputValue != UseManualAudioInputAttribute->GetBool());
                            Feature->bUseManualAudioInputValue = UseManualAudioInputAttribute->GetBool();
                        }
                        if (UseManualAudioOutputAttribute != nullptr)
                        {
                            bUpdated = bUpdated || (Feature->bUseManualAudioOutputValue !=
                                                    UseManualAudioOutputAttribute->GetBool());
                            Feature->bUseManualAudioOutputValue = UseManualAudioOutputAttribute->GetBool();
                        }
                        if (LocalAudioDeviceInputStartsMutedAttribute != nullptr)
                        {
                            bUpdated = bUpdated || (Feature->bLocalAudioDeviceInputStartsMutedValue !=
                                                    LocalAudioDeviceInputStartsMutedAttribute->GetBool());
                            Feature->bLocalAudioDeviceInputStartsMutedValue =
                                LocalAudioDeviceInputStartsMutedAttribute->GetBool();
                        }
                    }
                }
                return FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult(
                    FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::ERequireRoomUpdate::No,
                    bUpdated ? FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::Yes
                             : FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::No);
            })
        .ApplyFeatureRequestOnCreateRoomOperation([](const FRoomProviderWriteOperationRef &WriteOp,
                                                     const TSharedRef<FVoiceChatRoomFeatureRequest> &FeatureRequest) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            auto CreateOp = StaticCastSharedRef<FLobbyRoomProviderCreateOperation>(WriteOp);
            auto &Options = CreateOp->GetCreateLobbyOptions();
            Options.bEnableRTCRoom = true;
            Options.RTCRoomJoinFlags = FeatureRequest->JoinFlags();
            Options.bRTCRoomUseManualAudioInput = FeatureRequest->UseManualAudioInput();
            Options.bRTCRoomUseManualAudioOutput = FeatureRequest->UseManualAudioOutput();
            Options.bRTCRoomLocalAudioDeviceInputStartsMuted = FeatureRequest->LocalAudioDeviceInputStartsMuted();
            return Redpoint::EOS::API::Errors::Success();
        });

    // @todo: Crossplay feature.

    // Compute and apply the permission level across the PresenceRoomFeature and PublicAdvertisementRoomFeature.
    auto SetPermission = [](const FRoomProviderWriteOperationRef &WriteOp,
                            const TMap<FName, TSharedRef<IRoomFeatureRequestBase>> &FeatureRequests) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        auto UpdateOp = StaticCastSharedRef<FLobbyRoomProviderUpdateOperation>(WriteOp);
        if (UpdateOp->GetLobbyModification().IsValid())
        {
            EOS_ELobbyPermissionLevel PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_INVITEONLY;
            {
                auto PresenceRoomFeatureBase = FeatureRequests.Find(IPresenceRoomFeature::GetFeatureName());
                if (PresenceRoomFeatureBase != nullptr)
                {
                    auto PresenceRoomFeature =
                        StaticCastSharedRef<FPresenceRoomFeatureRequest>(*PresenceRoomFeatureBase);
                    if (PresenceRoomFeature->IsValid() && PresenceRoomFeature->AllowJoinByPresence())
                    {
                        PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_JOINVIAPRESENCE;
                    }
                }
            }
            {
                auto PublicAdvertisementFeatureBase =
                    FeatureRequests.Find(IPublicAdvertisementRoomFeature::GetFeatureName());
                if (PublicAdvertisementFeatureBase != nullptr)
                {
                    auto PublicAdvertisementFeature =
                        StaticCastSharedRef<FPublicAdvertisementRoomFeatureRequest>(*PublicAdvertisementFeatureBase);
                    if (PublicAdvertisementFeature->IsValid())
                    {
                        PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
                    }
                }
            }
            auto Result = FSetPermissionLevel::Execute(
                UpdateOp->GetLobbyModification().ToSharedRef(),
                FSetPermissionLevel::Options{PermissionLevel});
#if !NO_LOGGING
            if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
            {
                auto OperationObject = UpdateOp->AddUpdateToDebugJson("SetPermissionLevel", Result);
                switch (PermissionLevel)
                {
                case EOS_ELobbyPermissionLevel::EOS_LPL_INVITEONLY:
                    OperationObject->SetStringField("PermissionLevel", "InviteOnly");
                    break;
                case EOS_ELobbyPermissionLevel::EOS_LPL_JOINVIAPRESENCE:
                    OperationObject->SetStringField("PermissionLevel", "JoinViaPresence");
                    break;
                case EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED:
                    OperationObject->SetStringField("PermissionLevel", "PublicAdvertised");
                    break;
                default:
                    OperationObject->SetStringField("PermissionLevel", "Unknown");
                    break;
                }
            }
#endif
            return Redpoint::EOS::API::ConvertError(Result);
        }
        return Redpoint::EOS::API::Errors::Success();
    };
    this->RegisterFeatureSetHandler()
        .ApplyFeatureRequestSetOnPostCreateRoomOperation(
            [SetPermission](
                const FRoomProviderWriteOperationRef &WriteOp,
                const TMap<FName, TSharedRef<IRoomFeatureRequestBase>> &FeatureRequests,
                const TSet<FName> &DeletedFeatureRequests) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                return SetPermission(WriteOp, FeatureRequests);
            })
        .ApplyFeatureRequestSetOnUpdateRoomOperation(
            [SetPermission](
                const FRoomProviderWriteOperationRef &WriteOp,
                const TMap<FName, TSharedRef<IRoomFeatureRequestBase>> &FeatureRequests,
                const TMap<FName, TSharedRef<IRoomFeature>> &ExistingFeatures,
                const TSet<FName> &DeletedFeatureRequests) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                return SetPermission(WriteOp, FeatureRequests);
            });
}

void FLobbyRoomProvider::RegisterEvents()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;

    // 1. Need to get all of the FLobbyRoom instances from the owning IRoomSystem that match the ID
    // 2. Call SynchroniseRoom to update all the attributes in the FLobbyRoom based on EOS
    // 3. Call OnRoomUpdated on the IRoomSystem for the updated FLobbyRoom
    this->OnNativeLobbyUpdateReceivedEventHandle = FNotifyLobbyUpdateReceived::Register(
        this->PlatformHandle,
        FNotifyLobbyUpdateReceived::Options{},
        FNotifyLobbyUpdateReceived::EventDelegate::CreateSP(this, &FLobbyRoomProvider::OnNativeLobbyUpdateReceived));

    // 1. Need to get all of the FLobbyRoom instances from the owning IRoomSystem that match the ID
    // 2. Verify that the room has the IMemberListRoomFeature; if it doesn't, ignore the room
    // 3. Call SynchroniseRoom to update all the member attributes in the FLobbyRoom based on EOS
    // 4. Call the event on IMemberListRoomFeature for member data updates
    this->OnNativeLobbyMemberUpdateReceivedEventHandle = FNotifyLobbyMemberUpdateReceived::Register(
        this->PlatformHandle,
        FNotifyLobbyMemberUpdateReceived::Options{},
        FNotifyLobbyMemberUpdateReceived::EventDelegate::CreateSP(
            this,
            &FLobbyRoomProvider::OnNativeLobbyMemberUpdateReceived));

    // 1. Need to get all of the FLobbyRoom instances from the owning IRoomSystem that match the ID
    // 2. Verify that the room has the IMemberListRoomFeature; if it doesn't, ignore the room
    // 3. Call SynchroniseRoom to update all the member statuses in the FLobbyRoom based on EOS
    // 4. Call the event on IMemberListRoomFeature for member status change
    this->OnNativeLobbyMemberStatusReceivedEventHandle = FNotifyLobbyMemberStatusReceived::Register(
        this->PlatformHandle,
        FNotifyLobbyMemberStatusReceived::Options{},
        FNotifyLobbyMemberStatusReceived::EventDelegate::CreateSP(
            this,
            &FLobbyRoomProvider::OnNativeLobbyMemberStatusReceived));

    // 1. Load the lobby details for the InviteId to make a IRoomSearchResult
    // 2. Fire whatever event exists on IRoomSystem for incoming invites
    this->OnNativeLobbyInviteReceivedEventHandle = FNotifyLobbyInviteReceived::Register(
        this->PlatformHandle,
        FNotifyLobbyInviteReceived::Options{},
        FNotifyLobbyInviteReceived::EventDelegate::CreateSP(this, &FLobbyRoomProvider::OnNativeLobbyInviteReceived));

    // 1. Load the lobby details for the InviteId to make an IRoomSearchResult
    // 2. Fire whatever event exists on IRoomSystem for accepted invites
    this->OnNativeLobbyInviteAcceptedEventHandle = FNotifyLobbyInviteAccepted::Register(
        this->PlatformHandle,
        FNotifyLobbyInviteAccepted::Options{},
        FNotifyLobbyInviteAccepted::EventDelegate::CreateSP(this, &FLobbyRoomProvider::OnNativeLobbyInviteAccepted));

    // 1. Load the lobby details for the InviteId to make an IRoomSearchResult
    // 2. Fire whatever event exists on IRoomSystem for rejected invites
    this->OnNativeLobbyInviteRejectedEventHandle = FNotifyLobbyInviteRejected::Register(
        this->PlatformHandle,
        FNotifyLobbyInviteRejected::Options{},
        FNotifyLobbyInviteRejected::EventDelegate::CreateSP(this, &FLobbyRoomProvider::OnNativeLobbyInviteRejected));

    // 1. Load the lobby details for the UiEventId to make a IRoomSearchResult
    // 2. Join the lobby as we would join any other search result
    // 3. Once joined, make sure EOS_UI_AcknowledgeEventId is called for the event UI
    //    and pass the result code of the lobby join operation
    this->OnNativeJoinLobbyAcceptedEventHandle = FNotifyJoinLobbyAccepted::Register(
        this->PlatformHandle,
        FNotifyJoinLobbyAccepted::Options{},
        FNotifyJoinLobbyAccepted::EventDelegate::CreateSP(this, &FLobbyRoomProvider::OnNativeJoinLobbyAccepted));

    // EOS_Lobby_AddNotifySendLobbyNativeInviteRequested
    // - Unused because we do not turn on IMPI.

    // EOS_Lobby_AddNotifyRTCRoomConnectionChanged
    // - Propagate state to IVoiceChatRoomFeature and fire events on it.
    // - Make sure that if an RTC room is disconnected, that the VoiceChat module
    //   no longer shows the user as being in the channel.

    // 1. Perform the LeaveLobby operation.
    this->OnNativeLeaveLobbyRequestedEventHandle = FNotifyLeaveLobbyRequested::Register(
        this->PlatformHandle,
        FNotifyLeaveLobbyRequested::Options{},
        FNotifyLeaveLobbyRequested::EventDelegate::CreateSP(this, &FLobbyRoomProvider::OnNativeLeaveLobbyRequested));
}

void FLobbyRoomProvider::OnNativeLobbyUpdateReceived(
    const Redpoint::EOS::API::Lobby::FNotifyLobbyUpdateReceived::Result &Result)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    this->RoomSystemImpl->GetSynchronisationContext()->AddSynchronisationEvent(
        MakeShared<FRoomId>(LobbyRoomProviderName, Result.LobbyId),
        MakeShared<
            Synchronisation::FSynchronisationEvent<Redpoint::EOS::API::Lobby::FNotifyLobbyUpdateReceived::Result>>(
            "LobbyUpdate",
            Result));
}

void FLobbyRoomProvider::OnNativeLobbyMemberUpdateReceived(
    const Redpoint::EOS::API::Lobby::FNotifyLobbyMemberUpdateReceived::Result &Result)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    this->RoomSystemImpl->GetSynchronisationContext()->AddSynchronisationEvent(
        MakeShared<FRoomId>(LobbyRoomProviderName, Result.LobbyId),
        MakeShared<Synchronisation::FSynchronisationEvent<
            Redpoint::EOS::API::Lobby::FNotifyLobbyMemberUpdateReceived::Result>>("LobbyMemberUpdate", Result));
}

void FLobbyRoomProvider::OnNativeLobbyMemberStatusReceived(
    const Redpoint::EOS::API::Lobby::FNotifyLobbyMemberStatusReceived::Result &Result)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    this->RoomSystemImpl->GetSynchronisationContext()->AddSynchronisationEvent(
        MakeShared<FRoomId>(LobbyRoomProviderName, Result.LobbyId),
        MakeShared<Synchronisation::FSynchronisationEvent<
            Redpoint::EOS::API::Lobby::FNotifyLobbyMemberStatusReceived::Result>>("LobbyMemberStatus", Result));
    switch (Result.CurrentStatus)
    {
    case EOS_ELobbyMemberStatus::EOS_LMS_LEFT:
    case EOS_ELobbyMemberStatus::EOS_LMS_DISCONNECTED:
    case EOS_ELobbyMemberStatus::EOS_LMS_KICKED:
    case EOS_ELobbyMemberStatus::EOS_LMS_CLOSED:
        this->RoomSystemImpl->GetSynchronisationContext()->RecordMemberEvictedFromRoom(
            MakeShared<FRoomId>(LobbyRoomProviderName, Result.LobbyId),
            GetAccountId(Result.TargetUserId));
        break;
    case EOS_ELobbyMemberStatus::EOS_LMS_JOINED:
    case EOS_ELobbyMemberStatus::EOS_LMS_PROMOTED:
        break;
    default:
        checkf(false, TEXT("Unhandled EOS_ELobbyMemberStatus::EOS_LMS_... value!"));
        break;
    }
}

void FLobbyRoomProvider::OnNativeLobbyInviteReceived(
    const Redpoint::EOS::API::Lobby::FNotifyLobbyInviteReceived::Result &Result)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    EOS_EResult CopyLobbyDetailsResultCode;
    auto LobbyDetails = FCopyLobbyDetailsHandleByInviteId::Execute(
        this->PlatformHandle,
        FCopyLobbyDetailsHandleByInviteId::Options{Result.InviteId},
        CopyLobbyDetailsResultCode);
    if (CopyLobbyDetailsResultCode != EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("%s"),
            *ConvertError(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 TEXT("Failed to copy the lobby details handle by invite ID in OnNativeLobbyInviteReceived; this lobby "
                      "invite will not be surfaced in the game."),
                 CopyLobbyDetailsResultCode)
                 .ToLogString());
        return;
    }

    auto RoomSearchResult = this->CreateRoomSearchResultFromReadOperation(
        GetAccountId(Result.LocalUserId),
        MakeShared<FLobbyRoomProviderReadOperation>(GetAccountId(Result.LocalUserId), LobbyDetails, true),
        TEXT("OnNativeLobbyInviteReceived"),
        FString::Printf(TEXT("incoming room invite")),
        TOptional<EOS_UI_EventId>());
    if (!RoomSearchResult.IsValid())
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("Failed to convert the incoming invite's lobby details into a room search result for propagation "
                 "to the game; this lobby invite will not be surfaced in the game."));
        return;
    }

    this->OnRoomInviteLocallyReceivedFromSDKEventValue.ExecuteIfBound(
        Result.InviteId,
        GetAccountId(Result.TargetUserId),
        GetAccountId(Result.LocalUserId),
        RoomSearchResult.ToSharedRef());
}

void FLobbyRoomProvider::OnNativeLobbyInviteAccepted(
    const Redpoint::EOS::API::Lobby::FNotifyLobbyInviteAccepted::Result &Result)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    this->OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEventValue.ExecuteIfBound(
        Result.InviteId,
        GetAccountId(Result.LocalUserId));
}

void FLobbyRoomProvider::OnNativeLobbyInviteRejected(
    const Redpoint::EOS::API::Lobby::FNotifyLobbyInviteRejected::Result &Result)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    this->OnRoomInviteLocallyFullyRejectedFromSDKEventValue.ExecuteIfBound(
        Result.InviteId,
        GetAccountId(Result.LocalUserId));
}

void FLobbyRoomProvider::OnNativeJoinLobbyAccepted(
    const Redpoint::EOS::API::Lobby::FNotifyJoinLobbyAccepted::Result &Result)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    EOS_EResult ResultCode;
    auto LobbyDetails = FCopyLobbyDetailsHandleByUiEventId::Execute(
        this->PlatformHandle,
        FCopyLobbyDetailsHandleByUiEventId::Options{Result.UiEventId},
        ResultCode);
    if (ResultCode != EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Error,
            TEXT("%s"),
            *ConvertError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Unable to copy lobby details by UI event ID."), ResultCode)
                 .ToLogString());
        return;
    }

    auto ReadOp = MakeShared<FLobbyRoomProviderReadOperation>(GetAccountId(Result.LocalUserId), LobbyDetails, true);

    auto SearchResult = this->CreateRoomSearchResultFromReadOperation(
        GetAccountId(Result.LocalUserId),
        ReadOp,
        TEXT("OnNativeJoinLobbyAccepted"),
        FString::Printf(TEXT("overlay join operation")),
        Result.UiEventId);
    if (!SearchResult.IsValid())
    {
        UE_LOG(LogRedpointEOSRooms, Error, TEXT("Unable to create search result from UI event ID."));
        return;
    }

    this->OnRoomJoinRequestedFromSDKEventValue.ExecuteIfBound(
        GetAccountId(Result.LocalUserId),
        SearchResult.ToSharedRef());
}

void FLobbyRoomProvider::OnNativeLeaveLobbyRequested(
    const Redpoint::EOS::API::Lobby::FNotifyLeaveLobbyRequested::Result &Result)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    this->OnRoomLeaveRequestedFromSDKEventValue.ExecuteIfBound(
        GetAccountId(Result.LocalUserId),
        MakeShared<FRoomId>(LobbyRoomProviderName, Result.LobbyId));
}

FName FLobbyRoomProvider::GetProviderName() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return LobbyRoomProviderName;
}

FRoomProviderWriteOperationRef FLobbyRoomProvider::CreateCreateRoomOperation(
    const FRoomUserId &UserId,
    const FName &RoomNamespace)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return MakeShared<FLobbyRoomProviderCreateOperation>(UserId, RoomNamespace);
}

void FLobbyRoomProvider::ExecuteCreateRoomOperation(
    const FRoomProviderWriteOperationRef &CreateOperation,
    const FOnExecuteCreateRoomOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto CreateOp = StaticCastSharedRef<FLobbyRoomProviderCreateOperation>(CreateOperation);
    auto UserId = GetAccountId(CreateOp->GetCreateLobbyOptions().LocalUserId);
    auto Options = CreateOp->GetCreateLobbyOptions();

    // Force the lobby to be unjoinable and only have one member. These settings
    // will get changed during post-create, but are forced for now to prevent anyone
    // joining the lobby while it's still being set up.
    Options.MaxLobbyMembers = 1;
    Options.bAllowInvites = false;
    Options.PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_INVITEONLY;

    FCreateLobby::Execute(
        this->PlatformHandle,
        Options,
        FCreateLobby::CompletionDelegate::CreateSPLambda(
            this,
            [this, UserId, OnComplete, CreateOp](const FCreateLobby::Result &Result) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                // Check that the operation succeeded.
                if (Result.ResultCode != EOS_EResult::EOS_Success)
                {
                    OnComplete.ExecuteIfBound(
                        ConvertError(
                            ANSI_TO_TCHAR(__FUNCTION__),
                            TEXT("Failed to create the lobby as part of ExecuteCreateRoomOperation."),
                            Result.ResultCode),
                        nullptr);
                    return;
                }

                // Copy the details handle from the lobby we just created.
                EOS_EResult CopyLobbyDetailsResultCode;
                auto LobbyDetails = FCopyLobbyDetailsHandle::Execute(
                    this->PlatformHandle,
                    FCopyLobbyDetailsHandle::Options{GetProductUserId(UserId), Result.LobbyId},
                    CopyLobbyDetailsResultCode);
                if (CopyLobbyDetailsResultCode != EOS_EResult::EOS_Success)
                {
                    // Leave the lobby immediately as we can't even return a failing post-create operation
                    // object without the lobby details handle.
                    FLeaveLobby::Execute(
                        this->PlatformHandle,
                        FLeaveLobby::Options{GetProductUserId(UserId), Result.LobbyId},
                        FLeaveLobby::CompletionDelegate::CreateSPLambda(
                            this,
                            [OnComplete, Result](const FLeaveLobby::Result &) {
                                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                                OnComplete.ExecuteIfBound(
                                    ConvertError(
                                        ANSI_TO_TCHAR(__FUNCTION__),
                                        TEXT("Failed to copy the lobby details handle in ExecuteCreateRoomOperation "
                                             "after creating the lobby."),
                                        Result.ResultCode),
                                    nullptr);
                            }));
                    return;
                }

                // Create a modification handle so we can update the lobby in post-create operation.
                EOS_EResult UpdateLobbyModificationResultCode;
                auto UpdateLobbyModificationHandle = FUpdateLobbyModification::Execute(
                    this->PlatformHandle,
                    FUpdateLobbyModification::Options{GetProductUserId(UserId), Result.LobbyId},
                    UpdateLobbyModificationResultCode);
                if (UpdateLobbyModificationResultCode != EOS_EResult::EOS_Success)
                {
                    // Return a post-create operation that has effectively already failed. No calls
                    // on this type of FLobbyRoomProviderUpdateOperation will do anything, but
                    // ExecutePostCreateRoomOperation will propagate the error contained within it,
                    // after which the room system will call CancelPostCreateRoomOperation to clean up.
                    OnComplete.ExecuteIfBound(
                        Redpoint::EOS::API::Errors::Success(),
                        MakeShared<FLobbyRoomProviderUpdateOperation>(
                            UserId,
                            Result.LobbyId,
                            CreateOp->GetRoomNamespace(),
                            LobbyDetails,
                            UpdateLobbyModificationResultCode));
                    return;
                }

                // Return the post-create operation.
                OnComplete.ExecuteIfBound(
                    Redpoint::EOS::API::Errors::Success(),
                    MakeShared<FLobbyRoomProviderUpdateOperation>(
                        UserId,
                        Result.LobbyId,
                        CreateOp->GetRoomNamespace(),
                        LobbyDetails,
                        UpdateLobbyModificationHandle));
            }));
}

void FLobbyRoomProvider::ExecutePostCreateRoomOperation(
    const FRoomProviderWriteOperationRef &PostCreateOperation,
    const FOnExecutePostCreateRoomOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto UpdateOp = StaticCastSharedRef<FLobbyRoomProviderUpdateOperation>(PostCreateOperation);
    auto UserId = UpdateOp->GetActorUserId();
    auto LobbyId = UpdateOp->GetLobbyId();

    if (!UpdateOp->GetLobbyModification().IsValid())
    {
        // This operation is already in a failure state because we couldn't get the modification handle.
        OnComplete.ExecuteIfBound(
            ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Failed to create lobby modification handle in ExecuteCreateRoomOperation after creating the "
                     "lobby."),
                UpdateOp->GetUpdateLobbyModificationResultCode()),
            nullptr);
        return;
    }

    FUpdateLobby::Execute(
        this->PlatformHandle,
        FUpdateLobby::Options{UpdateOp->GetLobbyModification().ToSharedRef()},
        FUpdateLobby::CompletionDelegate::CreateSPLambda(
            this,
            [this, UserId, LobbyId, OnComplete](const FUpdateLobby::Result &Result) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                // Check that the operation succeeded.
                if (Result.ResultCode != EOS_EResult::EOS_Success && Result.ResultCode != EOS_EResult::EOS_NoChange)
                {
                    OnComplete.ExecuteIfBound(
                        ConvertError(
                            ANSI_TO_TCHAR(__FUNCTION__),
                            TEXT("Failed to update the lobby as part of ExecutePostCreateRoomOperation."),
                            Result.ResultCode),
                        nullptr);
                    return;
                }

                // Create the lobby room that wraps our underlying lobby.
                TSharedRef<FLobbyRoom> LobbyRoom = MakeShared<FLobbyRoom>(this->PlatformHandle, UserId, LobbyId);

                // Return the newly created lobby room.
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success(), LobbyRoom);
            }));
}

void FLobbyRoomProvider::CancelPostCreateRoomOperation(
    const FRoomProviderWriteOperationRef &PostCreateOperation,
    const FOnCancelPostCreateRoomOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto UpdateOp = StaticCastSharedRef<FLobbyRoomProviderUpdateOperation>(PostCreateOperation);
    auto UserId = UpdateOp->GetActorUserId();
    auto LobbyId = UpdateOp->GetLobbyId();

    FLeaveLobby::Execute(
        this->PlatformHandle,
        FLeaveLobby::Options{GetProductUserId(UserId), LobbyId},
        FLeaveLobby::CompletionDelegate::CreateSPLambda(this, [OnComplete](const FLeaveLobby::Result &Result) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            if (Result.ResultCode != EOS_EResult::EOS_Success)
            {
                OnComplete.ExecuteIfBound(
                    ConvertError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Failed to leave the lobby."), Result.ResultCode));
            }
            else
            {
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
            }
        }));
}

FRoomProviderWriteOperationRef FLobbyRoomProvider::CreateUpdateRoomOperation(
    const FRoomUserId &UserId,
    const FRoomRef &Room)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    auto LobbyRoom = StaticCastSharedRef<FLobbyRoom>(Room);

    // Create a modification handle so we can update the lobby.
    EOS_EResult UpdateLobbyModificationResultCode;
    auto UpdateLobbyModificationHandle = FUpdateLobbyModification::Execute(
        this->PlatformHandle,
        FUpdateLobbyModification::Options{GetProductUserId(UserId), LobbyRoom->GetRoomId()->GetRoomId()},
        UpdateLobbyModificationResultCode);
    if (UpdateLobbyModificationResultCode != EOS_EResult::EOS_Success)
    {
        // Return an update operation that has already failed.
        return MakeShared<FLobbyRoomProviderUpdateOperation>(
            UserId,
            LobbyRoom->GetRoomId()->GetRoomId(),
            LobbyRoom->GetRoomNamespace(),
            LobbyRoom->GetLobbyDetails(),
            UpdateLobbyModificationResultCode);
    }

    // Return the update operation.
    return MakeShared<FLobbyRoomProviderUpdateOperation>(
        UserId,
        LobbyRoom->GetRoomId()->GetRoomId(),
        LobbyRoom->GetRoomNamespace(),
        LobbyRoom->GetLobbyDetails(),
        UpdateLobbyModificationHandle);
}

void FLobbyRoomProvider::ExecuteUpdateRoomOperation(
    const FRoomProviderWriteOperationRef &UpdateOperation,
    const FOnExecuteUpdateRoomOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto UpdateOp = StaticCastSharedRef<FLobbyRoomProviderUpdateOperation>(UpdateOperation);
    auto LobbyId = UpdateOp->GetLobbyId();

    if (!UpdateOp->GetLobbyModification().IsValid())
    {
        // This operation is already in a failure state because we couldn't get the modification handle.
        OnComplete.ExecuteIfBound(ConvertError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Failed to create lobby modification handle in CreateUpdateRoomOperation."),
            UpdateOp->GetUpdateLobbyModificationResultCode()));
        return;
    }

    FUpdateLobby::Execute(
        this->PlatformHandle,
        FUpdateLobby::Options{UpdateOp->GetLobbyModification().ToSharedRef()},
        FUpdateLobby::CompletionDelegate::CreateSPLambda(
            this,
            [LobbyId, OnComplete](const FUpdateLobby::Result &Result) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                // Check that the operation succeeded.
                if (Result.ResultCode != EOS_EResult::EOS_Success && Result.ResultCode != EOS_EResult::EOS_NoChange)
                {
                    OnComplete.ExecuteIfBound(ConvertError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Failed to update the lobby as part of ExecuteUpdateRoomOperation."),
                        Result.ResultCode));
                    return;
                }

                // Return a successful result.
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
            }));
}

FRoomProviderSearchOperationRef FLobbyRoomProvider::CreateSearchRoomsOperation(
    const FRoomUserId &UserId,
    TOptional<uint32> MaxResults)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    EOS_EResult ResultCode;
    auto LobbySearch = FCreateLobbySearch::Execute(
        this->PlatformHandle,
        FCreateLobbySearch::Options{MaxResults.Get(EOS_LOBBY_MAX_SEARCH_RESULTS)},
        ResultCode);
    if (ResultCode != EOS_EResult::EOS_Success)
    {
        return MakeShared<FLobbyRoomProviderSearchOperation>(UserId, ResultCode);
    }
    else
    {
        return MakeShared<FLobbyRoomProviderSearchOperation>(UserId, LobbySearch);
    }
}

void FLobbyRoomProvider::ExecuteSearchRoomsOperation(
    const FRoomProviderSearchOperationRef &SearchOperation,
    const FOnExecuteSearchRoomsOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    this->ExecuteSearchRoomsOperationWithTimeout(SearchOperation, OnComplete, 1);
}

void FLobbyRoomProvider::ExecuteSearchRoomsOperationWithTimeout(
    const FRoomProviderSearchOperationRef &SearchOperation,
    const FOnExecuteSearchRoomsOperationComplete &OnComplete,
    const float CurrentAttemptCount)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::LobbySearch;
    using namespace ::Redpoint::EOS::Core::Id;

    auto SearchOp = StaticCastSharedRef<FLobbyRoomProviderSearchOperation>(SearchOperation);
    auto UserId = SearchOp->GetActorUserId();

    if (!SearchOp->GetLobbySearch().IsValid())
    {
        // This operation is already in a failure state because we couldn't create the lobby search.
        OnComplete.ExecuteIfBound(
            ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Failed to create lobby search in CreateSearchRoomsOperation."),
                SearchOp->GetCreateSearchResultCode()),
            TArray<FRoomProviderReadOperationRef>());
        return;
    }

    FFind::Execute(
        SearchOp->GetLobbySearch().ToSharedRef(),
        FFind::Options{GetProductUserId(UserId)},
        FFind::CompletionDelegate::CreateSPLambda(
            this,
            [this, CurrentAttemptCount, SearchOperation, OnComplete, UserId](
                EOS_EResult ResultCode,
                const TArray<FFind::ResultEntry> &Handles) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                // Check that the operation succeeded.
                if (ResultCode == EOS_EResult::EOS_TooManyRequests || ResultCode == EOS_EResult::EOS_TimedOut)
                {
                    FTSTicker::GetCoreTicker().AddTicker(
                        FTickerDelegate::CreateSPLambda(
                            this,
                            [this, SearchOperation, OnComplete, CurrentAttemptCount](float) {
                                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                                this->ExecuteSearchRoomsOperationWithTimeout(
                                    SearchOperation,
                                    OnComplete,
                                    CurrentAttemptCount + 1);
                                return false;
                            }),
                        ResultCode == EOS_EResult::EOS_TooManyRequests
                            ? (60.0f + FMath::RandRange(0.0f, 15.0f))
                            : (static_cast<float>(CurrentAttemptCount) * 5.0f + FMath::RandRange(0.0f, 2.5f)));
                    return;
                }
                else if (ResultCode != EOS_EResult::EOS_Success)
                {
                    OnComplete.ExecuteIfBound(
                        ConvertError(
                            ANSI_TO_TCHAR(__FUNCTION__),
                            TEXT("Failed to search for lobbies as part of ExecuteSearchRoomsOperation."),
                            ResultCode),
                        TArray<FRoomProviderReadOperationRef>());
                    return;
                }

                // Generate all of the read operation objects and return them.
                TArray<FRoomProviderReadOperationRef> Results;
                for (const auto &Handle : Handles)
                {
                    Results.Add(MakeShared<FLobbyRoomProviderReadOperation>(UserId, Handle, true));
                }
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success(), Results);
            }));
}

FRoomProviderWriteOperationRef FLobbyRoomProvider::CreateJoinRoomByIdOperation(
    const FRoomUserId &UserId,
    const FRoomIdRef &RoomId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return MakeShared<FLobbyRoomProviderJoinOperation>(UserId, RoomId->GetRoomId());
}

FRoomProviderWriteOperationRef FLobbyRoomProvider::CreateJoinRoomBySearchResultOperation(
    const FRoomUserId &UserId,
    const FRoomSearchResultRef &RoomSearchResult)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    auto RoomSearchResultImpl = StaticCastSharedRef<FRoomSearchResultImpl>(RoomSearchResult);
    auto ReadOperation = RoomSearchResultImpl->GetReadOperation();
    auto LobbyReadOperation = StaticCastSharedRef<FLobbyRoomProviderReadOperation>(ReadOperation);
    auto LobbyDetails = LobbyReadOperation->GetLobbyDetails();

    return MakeShared<FLobbyRoomProviderJoinOperation>(UserId, LobbyDetails);
}

void FLobbyRoomProvider::ExecuteJoinRoomOperation(
    const FRoomProviderWriteOperationRef &JoinOperation,
    const FOnExecuteJoinRoomOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto JoinOp = StaticCastSharedRef<FLobbyRoomProviderJoinOperation>(JoinOperation);
    auto UserId = GetAccountId(JoinOp->GetJoinLobbyByIdOptions().LocalUserId);

    auto AfterJoin = [this, OnComplete, UserId](EOS_EResult ResultCode, FString LobbyId) {
        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

        // Check that the operation succeeded.
        if (ResultCode != EOS_EResult::EOS_Success)
        {
            OnComplete.ExecuteIfBound(
                ConvertError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Failed to join the lobby as part of ExecuteJoinRoomOperation."),
                    ResultCode),
                nullptr);
            return;
        }

        // Copy the details handle from the lobby we just joined.
        EOS_EResult CopyLobbyDetailsResultCode;
        auto LobbyDetails = FCopyLobbyDetailsHandle::Execute(
            this->PlatformHandle,
            FCopyLobbyDetailsHandle::Options{GetProductUserId(UserId), LobbyId},
            CopyLobbyDetailsResultCode);
        if (CopyLobbyDetailsResultCode != EOS_EResult::EOS_Success)
        {
            // Leave the lobby immediately as we can't even return a failing post-join operation
            // object without the lobby details handle.
            FLeaveLobby::Execute(
                this->PlatformHandle,
                FLeaveLobby::Options{GetProductUserId(UserId), LobbyId},
                FLeaveLobby::CompletionDelegate::CreateSPLambda(
                    this,
                    [OnComplete, ResultCode](const FLeaveLobby::Result &) {
                        REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                        OnComplete.ExecuteIfBound(
                            ConvertError(
                                ANSI_TO_TCHAR(__FUNCTION__),
                                TEXT("Failed to copy the lobby details handle in ExecuteJoinRoomOperation "
                                     "after joining the lobby."),
                                ResultCode),
                            nullptr);
                    }));
            return;
        }

        // Create a modification handle so we can update the lobby in post-join operation.
        EOS_EResult UpdateLobbyModificationResultCode;
        auto UpdateLobbyModificationHandle = FUpdateLobbyModification::Execute(
            this->PlatformHandle,
            FUpdateLobbyModification::Options{GetProductUserId(UserId), LobbyId},
            UpdateLobbyModificationResultCode);
        if (UpdateLobbyModificationResultCode != EOS_EResult::EOS_Success)
        {
            // Return a post-join operation that has effectively already failed. No calls
            // on this type of FLobbyRoomProviderUpdateOperation will do anything, but
            // ExecutePostJoinRoomOperation will propagate the error contained within it,
            // after which the room system will call CancelPostJoinRoomOperation to clean up.
            OnComplete.ExecuteIfBound(
                Redpoint::EOS::API::Errors::Success(),
                MakeShared<FLobbyRoomProviderUpdateOperation>(
                    UserId,
                    LobbyId,
                    /* @note: Only way we could provide this is if we enforced room namespaces to be passed to
                       join-by-ID operations. */
                    NAME_None,
                    LobbyDetails,
                    UpdateLobbyModificationResultCode));
            return;
        }

        // Temporarily create a read operation so we can use GetAttributes and GetRoomNamespaceFromAttributesByKey
        // to then pass into the FLobbyRoomProviderUpdateOperation so that bucket feature handling can know the room
        // namespace.
        auto ReadOp = MakeShared<FLobbyRoomProviderReadOperation>(UserId, LobbyDetails, false);
        auto [Attrs, AttrsError] = ReadOp->GetAttributes();
        if (!AttrsError.WasSuccessful())
        {
            // Failed to read room attributes after join.
            OnComplete.ExecuteIfBound(
                Redpoint::EOS::API::Errors::Success(),
                MakeShared<FLobbyRoomProviderUpdateOperation>(
                    UserId,
                    LobbyId,
                    /* @note: Only way we could provide this is if we enforced room namespaces to be passed to
                       join-by-ID operations. */
                    NAME_None,
                    LobbyDetails,
                    AttrsError.ResultCode()));
            return;
        }
        auto RoomNamespace = this->GetRoomNamespaceFromAttributesByKey(Attrs);
        if (!RoomNamespace.IsSet())
        {
            // Failed to parse out room namespace (which makes it impossible to join this room).
            UE_LOG(
                LogRedpointEOSRooms,
                Error,
                TEXT("Attempted to join room that did not have valid room namespace attributes. The join operation "
                     "will fail."));
            OnComplete.ExecuteIfBound(
                Redpoint::EOS::API::Errors::Success(),
                MakeShared<FLobbyRoomProviderUpdateOperation>(
                    UserId,
                    LobbyId,
                    /* @note: Only way we could provide this is if we enforced room namespaces to be passed to
                       join-by-ID operations. */
                    NAME_None,
                    LobbyDetails,
                    EOS_EResult::EOS_NotFound));
            return;
        }

        // Return the post-join operation.
        OnComplete.ExecuteIfBound(
            Redpoint::EOS::API::Errors::Success(),
            MakeShared<FLobbyRoomProviderUpdateOperation>(
                UserId,
                LobbyId,
                RoomNamespace.GetValue(),
                LobbyDetails,
                UpdateLobbyModificationHandle));
    };

    if (JoinOp->IsJoinById())
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Verbose,
            TEXT("Running FJoinLobbyById operation as '%s'."),
            *GetUserIdString(GetAccountId(*(JoinOp->GetJoinLobbyByIdOptions().LocalUserId))));
        FJoinLobbyById::Execute(
            this->PlatformHandle,
            JoinOp->GetJoinLobbyByIdOptions(),
            FJoinLobbyById::CompletionDelegate::CreateSPLambda(this, [AfterJoin](const FJoinLobbyById::Result &Result) {
                AfterJoin(Result.ResultCode, Result.LobbyId);
            }));
    }
    else
    {
        UE_LOG(
            LogRedpointEOSRooms,
            Verbose,
            TEXT("Running FJoinLobby operation as '%s'."),
            *GetUserIdString(GetAccountId(*(JoinOp->GetJoinLobbyOptions().LocalUserId))));
        FJoinLobby::Execute(
            this->PlatformHandle,
            JoinOp->GetJoinLobbyOptions(),
            FJoinLobby::CompletionDelegate::CreateSPLambda(this, [AfterJoin](const FJoinLobby::Result &Result) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                AfterJoin(Result.ResultCode, Result.LobbyId);
            }));
    }
}

void FLobbyRoomProvider::ExecutePostJoinRoomOperation(
    const FRoomProviderWriteOperationRef &PostJoinOperation,
    const FOnExecutePostJoinRoomOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto UpdateOp = StaticCastSharedRef<FLobbyRoomProviderUpdateOperation>(PostJoinOperation);
    auto UserId = UpdateOp->GetActorUserId();
    auto LobbyId = UpdateOp->GetLobbyId();

    if (!UpdateOp->GetLobbyModification().IsValid())
    {
        // This operation is already in a failure state because we couldn't get the modification handle.
        OnComplete.ExecuteIfBound(
            ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Failed to create lobby modification handle in ExecuteCreateRoomOperation after joining the "
                     "lobby."),
                UpdateOp->GetUpdateLobbyModificationResultCode()),
            nullptr);
        return;
    }

    FUpdateLobby::Execute(
        this->PlatformHandle,
        FUpdateLobby::Options{UpdateOp->GetLobbyModification().ToSharedRef()},
        FUpdateLobby::CompletionDelegate::CreateSPLambda(
            this,
            [this, UserId, LobbyId, OnComplete](const FUpdateLobby::Result &Result) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                // Check that the operation succeeded.
                if (Result.ResultCode != EOS_EResult::EOS_Success && Result.ResultCode != EOS_EResult::EOS_NoChange)
                {
                    OnComplete.ExecuteIfBound(
                        ConvertError(
                            ANSI_TO_TCHAR(__FUNCTION__),
                            TEXT("Failed to update the lobby as part of ExecutePostJoinRoomOperation."),
                            Result.ResultCode),
                        nullptr);
                    return;
                }

                // Create the lobby room that wraps our underlying lobby.
                TSharedRef<FLobbyRoom> LobbyRoom = MakeShared<FLobbyRoom>(this->PlatformHandle, UserId, LobbyId);

                // Return the newly joined lobby room.
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success(), LobbyRoom);
            }));
}

void FLobbyRoomProvider::CancelPostJoinRoomOperation(
    const FRoomProviderWriteOperationRef &PostJoinOperation,
    const FOnCancelPostJoinRoomOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto UpdateOp = StaticCastSharedRef<FLobbyRoomProviderUpdateOperation>(PostJoinOperation);
    auto UserId = UpdateOp->GetActorUserId();
    auto LobbyId = UpdateOp->GetLobbyId();

    FLeaveLobby::Execute(
        this->PlatformHandle,
        FLeaveLobby::Options{GetProductUserId(UserId), LobbyId},
        FLeaveLobby::CompletionDelegate::CreateSPLambda(this, [OnComplete](const FLeaveLobby::Result &Result) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            if (Result.ResultCode != EOS_EResult::EOS_Success)
            {
                OnComplete.ExecuteIfBound(
                    ConvertError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Failed to leave the lobby."), Result.ResultCode));
            }
            else
            {
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
            }
        }));
}

FRoomProviderActionOperationRef FLobbyRoomProvider::CreateLeaveRoomOperation(
    const FRoomUserId &UserId,
    const FRoomRef &Room)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return MakeShared<FLobbyRoomProviderLeaveOperation>(UserId, Room->GetRoomId()->GetRoomId());
}

void FLobbyRoomProvider::ExecuteLeaveRoomOperation(
    const FRoomProviderActionOperationRef &LeaveOperation,
    const FOnExecuteLeaveRoomOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto LeaveOp = StaticCastSharedRef<FLobbyRoomProviderLeaveOperation>(LeaveOperation);
    auto UserId = LeaveOp->GetActorUserId();
    auto LobbyId = LeaveOp->GetLobbyId();

    FLeaveLobby::Execute(
        this->PlatformHandle,
        FLeaveLobby::Options{GetProductUserId(UserId), LobbyId},
        FLeaveLobby::CompletionDelegate::CreateSPLambda(this, [OnComplete](const FLeaveLobby::Result &Result) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            if (Result.ResultCode != EOS_EResult::EOS_Success)
            {
                OnComplete.ExecuteIfBound(
                    ConvertError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Failed to leave the lobby."), Result.ResultCode));
            }
            else
            {
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
            }
        }));
}

FRoomProviderActionOperationRef FLobbyRoomProvider::CreatePromoteRoomMemberOperation(
    const FRoomUserId &UserId,
    const FRoomRef &Room,
    const FRoomUserId &TargetUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return MakeShared<FLobbyRoomProviderTargetMemberOperation>(UserId, Room->GetRoomId()->GetRoomId(), TargetUserId);
}

void FLobbyRoomProvider::ExecutePromoteRoomMemberOperation(
    const FRoomProviderActionOperationRef &PromoteOperation,
    const FOnExecutePromoteRoomMemberOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto PromoteOp = StaticCastSharedRef<FLobbyRoomProviderTargetMemberOperation>(PromoteOperation);
    auto UserId = PromoteOp->GetActorUserId();
    auto LobbyId = PromoteOp->GetLobbyId();
    auto TargetUserId = PromoteOp->GetTargetUserId();

    FPromoteMember::Execute(
        this->PlatformHandle,
        FPromoteMember::Options{GetProductUserId(UserId), LobbyId, GetProductUserId(TargetUserId)},
        FPromoteMember::CompletionDelegate::CreateSPLambda(this, [OnComplete](const FPromoteMember::Result &Result) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            if (Result.ResultCode != EOS_EResult::EOS_Success)
            {
                OnComplete.ExecuteIfBound(ConvertError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Failed to promote the member to the owner of the lobby."),
                    Result.ResultCode));
            }
            else
            {
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
            }
        }));
}

FRoomProviderActionOperationRef FLobbyRoomProvider::CreateKickRoomMemberOperation(
    const FRoomUserId &UserId,
    const FRoomRef &Room,
    const FRoomUserId &TargetUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return MakeShared<FLobbyRoomProviderTargetMemberOperation>(UserId, Room->GetRoomId()->GetRoomId(), TargetUserId);
}

void FLobbyRoomProvider::ExecuteKickRoomMemberOperation(
    const FRoomProviderActionOperationRef &KickOperation,
    const FOnExecuteKickRoomMemberOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto KickOp = StaticCastSharedRef<FLobbyRoomProviderTargetMemberOperation>(KickOperation);
    auto UserId = KickOp->GetActorUserId();
    auto LobbyId = KickOp->GetLobbyId();
    auto TargetUserId = KickOp->GetTargetUserId();

    // Explicitly record that we're intending to kick the target user. The FKickMember operation is an asynchronous
    // operation that runs over many frames, while synchronisation happens every frame. It is non-deterministic on which
    // frame the EOS SDK will remove the target user from our local list of members; sometimes it will happen on the
    // frame after FKickMember starts, other times it will happen many frames later. Due to this, it's not possible to
    // rely on AddSynchronisationEvent (since synchronisation may detect the member being removed before the EOS SDK
    // fires the "member updated" event), and it's not possible to simulate the event by calling AddSynchronisationEvent
    // because the events get cleared out every frame.
    //
    // Instead we have a dedicated API for telling the synchronisation context that we're anticipating that a kick of
    // the target player might happen, and we fallback to this list during synchronisation if there's no recorded
    // "LobbyMemberStatus" for a disappearing member. After the callback completes, we remove the anticipated kick
    // regardless of the outcome, since we should either have a recorded "LobbyMemberStatus" event or the kick should
    // already have happened while the synchronisation could rely on the anticipated kick list.
    this->RoomSystemImpl->GetSynchronisationContext()->AddAnticipatedKick(
        MakeShared<FRoomId>(LobbyRoomProviderName, LobbyId),
        UserId,
        TargetUserId);

    FKickMember::Execute(
        this->PlatformHandle,
        FKickMember::Options{GetProductUserId(UserId), LobbyId, GetProductUserId(TargetUserId)},
        FKickMember::CompletionDelegate::CreateSPLambda(
            this,
            [this, OnComplete, LobbyId, UserId, TargetUserId](const FKickMember::Result &Result) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                if (Result.ResultCode != EOS_EResult::EOS_Success)
                {
                    if (this->RoomSystemImpl != nullptr)
                    {
                        this->RoomSystemImpl->GetSynchronisationContext()->RemoveAnticipatedKick(
                            MakeShared<FRoomId>(LobbyRoomProviderName, LobbyId),
                            UserId,
                            TargetUserId);
                    }
                    OnComplete.ExecuteIfBound(ConvertError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Failed to kick the member from the lobby room."),
                        Result.ResultCode));
                }
                else
                {
                    // @note: RemoveAnticipatedKick is called when synchronisation reads it via HasAnticipatedKick.
                    OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
                }
            }));
}

FRoomProviderActionOperationRef FLobbyRoomProvider::CreateSendRoomInviteOperation(
    const FRoomUserId &UserId,
    const FRoomRef &Room,
    const FRoomUserId &TargetUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return MakeShared<FLobbyRoomProviderTargetMemberOperation>(UserId, Room->GetRoomId()->GetRoomId(), TargetUserId);
}

void FLobbyRoomProvider::ExecuteSendRoomInviteOperation(
    const FRoomProviderActionOperationRef &KickOperation,
    const FOnExecuteSendRoomInviteOperationComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto InviteOp = StaticCastSharedRef<FLobbyRoomProviderTargetMemberOperation>(KickOperation);
    auto UserId = InviteOp->GetActorUserId();
    auto LobbyId = InviteOp->GetLobbyId();
    auto TargetUserId = InviteOp->GetTargetUserId();

    FSendInvite::Execute(
        this->PlatformHandle,
        FSendInvite::Options{GetProductUserId(UserId), LobbyId, GetProductUserId(TargetUserId)},
        FSendInvite::CompletionDelegate::CreateSPLambda(this, [OnComplete](const FSendInvite::Result &Result) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            if (Result.ResultCode != EOS_EResult::EOS_Success)
            {
                OnComplete.ExecuteIfBound(ConvertError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Failed to send an invite for the lobby room to the target user."),
                    Result.ResultCode));
            }
            else
            {
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
            }
        }));
}

FRoomProviderReadOperationRef FLobbyRoomProvider::CreateReadRoomOperation(
    const FRoomUserId &UserId,
    const FRoomRef &Room)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return MakeShared<FLobbyRoomProviderReadOperation>(
        UserId,
        StaticCastSharedRef<FLobbyRoom>(Room)->GetLobbyDetails(),
        false);
}

void FLobbyRoomProvider::RestoreRoomInvites(
    const FRestoreRoomInvitesRequest &Request,
    const IRoomSystem::FOnRestoreRoomInvitesComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    FQueryInvites::Execute(
        this->PlatformHandle,
        FQueryInvites::Options{GetProductUserId(Request.LocalUser)},
        FQueryInvites::CompletionDelegate::CreateSPLambda(
            this,
            [this, Request, OnComplete](EOS_EResult ResultCode, const TArray<FString> &ResultEntries) {
                REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

                if (ResultCode != EOS_EResult::EOS_Success)
                {
                    OnComplete.ExecuteIfBound(
                        ConvertError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Failed to query invites."), ResultCode));
                    return;
                }

                for (const auto &InviteId : ResultEntries)
                {
                    EOS_EResult CopyLobbyDetailsResultCode;
                    auto LobbyDetails = FCopyLobbyDetailsHandleByInviteId::Execute(
                        this->PlatformHandle,
                        FCopyLobbyDetailsHandleByInviteId::Options{InviteId},
                        CopyLobbyDetailsResultCode);
                    if (CopyLobbyDetailsResultCode != EOS_EResult::EOS_Success)
                    {
                        UE_LOG(
                            LogRedpointEOSRooms,
                            Error,
                            TEXT("%s"),
                            *ConvertError(
                                 ANSI_TO_TCHAR(__FUNCTION__),
                                 TEXT("Failed to copy the lobby details handle by invite ID in RestoreRoomInvites; "
                                      "this lobby invite will not be surfaced in the game."),
                                 CopyLobbyDetailsResultCode)
                                 .ToLogString());
                        return;
                    }

                    auto RoomSearchResult = this->CreateRoomSearchResultFromReadOperation(
                        Request.LocalUser,
                        MakeShared<FLobbyRoomProviderReadOperation>(Request.LocalUser, LobbyDetails, true),
                        TEXT("RestoreRoomInvites"),
                        FString::Printf(TEXT("restored room invite")),
                        TOptional<EOS_UI_EventId>());
                    if (!RoomSearchResult.IsValid())
                    {
                        UE_LOG(
                            LogRedpointEOSRooms,
                            Error,
                            TEXT("Failed to convert the incoming invite's lobby details into a room search result for "
                                 "propagation to the game; this lobby invite will not be surfaced in the game."));
                        return;
                    }

                    UE_LOG(
                        LogRedpointEOSRooms,
                        Warning,
                        TEXT("Due to EOS SDK limitations, it is not possible to determine the sender of room invites "
                             "when restoring them at game startup. The sender of room invite '%s' will be reported as "
                             "an invalid user."),
                        *InviteId);
                    this->OnRoomInviteLocallyReceivedFromSDKEventValue
                        .ExecuteIfBound(InviteId, FRoomUserId(), Request.LocalUser, RoomSearchResult.ToSharedRef());
                }

                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
            }));
}

void FLobbyRoomProvider::RejectRoomInvite(
    const FRoomInviteRef &RoomInvite,
    const IRoomSystem::FOnRejectRoomInviteComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::API::Lobby;
    using namespace ::Redpoint::EOS::Core::Id;

    auto UserId = RoomInvite->GetRoomInviteRecipientUserId();
    auto InviteId = RoomInvite->GetRoomInviteId();

    FRejectInvite::Execute(
        this->PlatformHandle,
        FRejectInvite::Options{GetProductUserId(UserId), InviteId},
        FRejectInvite::CompletionDelegate::CreateSPLambda(this, [OnComplete](const FRejectInvite::Result &Result) {
            REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

            if (Result.ResultCode != EOS_EResult::EOS_Success)
            {
                OnComplete.ExecuteIfBound(
                    ConvertError(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Failed to reject an invite."), Result.ResultCode));
            }
            else
            {
                OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::Success());
            }
        }));
}

FRoomProvider::FOnRoomJoinRequestedFromSDKEvent &FLobbyRoomProvider::OnRoomJoinRequestedFromSDKEvent()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->OnRoomJoinRequestedFromSDKEventValue;
}

FRoomProvider::FOnRoomLeaveRequestedFromSDKEvent &FLobbyRoomProvider::OnRoomLeaveRequestedFromSDKEvent()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->OnRoomLeaveRequestedFromSDKEventValue;
}

FRoomProvider::FOnRoomInviteLocallyReceivedFromSDKEvent &FLobbyRoomProvider::OnRoomInviteLocallyReceivedFromSDKEvent()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->OnRoomInviteLocallyReceivedFromSDKEventValue;
}

FRoomProvider::FOnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent &FLobbyRoomProvider::
    OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEventValue;
}

FRoomProvider::FOnRoomInviteLocallyFullyRejectedFromSDKEvent &FLobbyRoomProvider::
    OnRoomInviteLocallyFullyRejectedFromSDKEvent()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->OnRoomInviteLocallyFullyRejectedFromSDKEventValue;
}

void FLobbyRoomProvider::Reset()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    FRoomProvider::Reset();

    this->RoomSystemImpl = nullptr;
    this->OnRoomJoinRequestedFromSDKEventValue.Unbind();
    this->OnRoomLeaveRequestedFromSDKEventValue.Unbind();
    this->OnRoomInviteLocallyReceivedFromSDKEventValue.Unbind();
    this->OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEventValue.Unbind();
    this->OnRoomInviteLocallyFullyRejectedFromSDKEventValue.Unbind();
    this->OnNativeLobbyUpdateReceivedEventHandle.Reset();
    this->OnNativeLobbyMemberUpdateReceivedEventHandle.Reset();
    this->OnNativeLobbyMemberStatusReceivedEventHandle.Reset();
    this->OnNativeLobbyInviteReceivedEventHandle.Reset();
    this->OnNativeLobbyInviteAcceptedEventHandle.Reset();
    this->OnNativeLobbyInviteRejectedEventHandle.Reset();
    this->OnNativeJoinLobbyAcceptedEventHandle.Reset();
    this->OnNativeLeaveLobbyRequestedEventHandle.Reset();
}

}

REDPOINT_EOS_CODE_GUARD_END()