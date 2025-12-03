// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartySystemRedpointEOS.h"

#include "Dom/JsonObject.h"
#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyJoinInfoRedpointEOSRoomId.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyJoinInfoRedpointEOSRoomInvite.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyMemberRedpointEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyRedpointEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/SubsystemNames.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeature.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeatureRequest.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(415509281, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::Error;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party;
using namespace ::Redpoint::EOS::Presence;

const FString FOnlinePartySystemRedpointEOS::DefaultPartyBucketId = TEXT("Parties");

typedef TTuple<FUniqueNetIdRef, TSharedRef<const FOnlinePartyIdRedpointEOS>> FUniqueNetIdAndPartyId;
typedef TTuple<FUniqueNetIdRef, TSharedRef<const FOnlinePartyIdRedpointEOS>, FUniqueNetIdRef>
    FUniqueNetIdAndPartyIdAndMemberId;
typedef TTuple<FUniqueNetIdRef, FOnlinePartyTypeId> FUniqueNetIdAndPartyTypeId;
static const auto CPartyDatabaseLocalUserId = FIndexedColumnDefinition<FUniqueNetIdRef>(
    FName(TEXT("LocalUserId")),
    [](const FUniqueNetIdRef &A, const FUniqueNetIdRef &B) {
        return *A == *B;
    },
    [](const FUniqueNetIdRef &K) {
        return GetTypeHash(*K);
    });
static const auto CPartyDatabaseLocalUserIdAndPartyId = FIndexedColumnDefinition<FUniqueNetIdAndPartyId>(
    FName(TEXT("LocalUserIdAndPartyId")),
    [](const FUniqueNetIdAndPartyId &A, const FUniqueNetIdAndPartyId &B) {
        return *A.Key == *B.Key && A.Value->ToString() == B.Value->ToString();
    },
    [](const FUniqueNetIdAndPartyId &K) {
        return HashCombine(GetTypeHash(*K.Key), GetTypeHash(K.Value->ToString()));
    });
static const auto CPartyDatabaseLocalUserIdAndPartyTypeId = FIndexedColumnDefinition<FUniqueNetIdAndPartyTypeId>(
    FName(TEXT("LocalUserIdAndPartyTypeId")),
    [](const FUniqueNetIdAndPartyTypeId &A, const FUniqueNetIdAndPartyTypeId &B) {
        return *A.Key == *B.Key && A.Value.GetValue() == B.Value.GetValue();
    },
    [](const FUniqueNetIdAndPartyTypeId &K) {
        return HashCombine(GetTypeHash(*K.Key), GetTypeHash(K.Value.GetValue()));
    });
static const auto CPartyDatabaseRoomRef = FIndexedColumnDefinition<FRoomRef>(
    FName(TEXT("RoomRef")),
    [](const FRoomRef &A, const FRoomRef &B) {
        return &A.Get() == &B.Get();
    },
    [](const FRoomRef &K) {
        return GetTypeHash(&K.Get());
    });
static const auto CPartyDatabaseOnlinePartyRedpointEOSRef =
    FIndexedColumnDefinition<TSharedRef<FOnlinePartyRedpointEOS>>(
        FName(TEXT("OnlinePartyRedpointEOSRef")),
        [](const TSharedRef<FOnlinePartyRedpointEOS> &A, const TSharedRef<FOnlinePartyRedpointEOS> &B) {
            return A == B;
        },
        [](const TSharedRef<FOnlinePartyRedpointEOS> &K) {
            return GetTypeHash(K);
        });
static const auto CPartyDatabasePartyMemberJoinedHandle = FIndexedColumnDefinition<TSharedRef<FDelegateHandle>>(
    FName(TEXT("PartyMemberJoinedHandle")),
    [](const TSharedRef<FDelegateHandle> &A, const TSharedRef<FDelegateHandle> &B) {
        return A == B;
    },
    [](const TSharedRef<FDelegateHandle> &K) {
        return GetTypeHash(K);
    });
static const auto CPartyDatabasePartyMemberLeftHandle = FIndexedColumnDefinition<TSharedRef<FDelegateHandle>>(
    FName(TEXT("PartyMemberLeftHandle")),
    [](const TSharedRef<FDelegateHandle> &A, const TSharedRef<FDelegateHandle> &B) {
        return A == B;
    },
    [](const TSharedRef<FDelegateHandle> &K) {
        return GetTypeHash(K);
    });
static const auto CPartyDatabasePartyMemberUpdatedHandle = FIndexedColumnDefinition<TSharedRef<FDelegateHandle>>(
    FName(TEXT("PartyMemberUpdatedHandle")),
    [](const TSharedRef<FDelegateHandle> &A, const TSharedRef<FDelegateHandle> &B) {
        return A == B;
    },
    [](const TSharedRef<FDelegateHandle> &K) {
        return GetTypeHash(K);
    });
static const auto CPartyDatabasePartyMemberPromotedHandle = FIndexedColumnDefinition<TSharedRef<FDelegateHandle>>(
    FName(TEXT("PartyMemberPromotedHandle")),
    [](const TSharedRef<FDelegateHandle> &A, const TSharedRef<FDelegateHandle> &B) {
        return A == B;
    },
    [](const TSharedRef<FDelegateHandle> &K) {
        return GetTypeHash(K);
    });
static const auto CPartyMemberDatabaseLocalUserIdAndPartyId = FIndexedColumnDefinition<FUniqueNetIdAndPartyId>(
    FName(TEXT("LocalUserIdAndPartyId")),
    [](const FUniqueNetIdAndPartyId &A, const FUniqueNetIdAndPartyId &B) {
        return *A.Key == *B.Key && A.Value->ToString() == B.Value->ToString();
    },
    [](const FUniqueNetIdAndPartyId &K) {
        return HashCombine(GetTypeHash(*K.Key), GetTypeHash(K.Value->ToString()));
    });
static const auto CPartyMemberDatabaseLocalUserIdAndPartyIdAndMemberId =
    FIndexedColumnDefinition<FUniqueNetIdAndPartyIdAndMemberId>(
        FName(TEXT("LocalUserIdAndPartyIdAndMemberId")),
        [](const FUniqueNetIdAndPartyIdAndMemberId &A, const FUniqueNetIdAndPartyIdAndMemberId &B) {
            return *A.Get<0>() == *B.Get<0>() && A.Get<1>()->ToString() == B.Get<1>()->ToString() &&
                   *A.Get<2>() == *B.Get<2>();
        },
        [](const FUniqueNetIdAndPartyIdAndMemberId &K) {
            return HashCombine(
                HashCombine(GetTypeHash(*K.Get<0>()), GetTypeHash(K.Get<1>()->ToString())),
                GetTypeHash(*K.Get<2>()));
        });
static const auto CPartyMemberDatabaseRoomMemberRef = FIndexedColumnDefinition<FRoomMemberRef>(
    FName(TEXT("RoomMemberRef")),
    [](const FRoomMemberRef &A, const FRoomMemberRef &B) {
        return &A.Get() == &B.Get();
    },
    [](const FRoomMemberRef &K) {
        return GetTypeHash(&K.Get());
    });
static const auto CPartyMemberDatabaseOnlinePartyMemberRedpointEOSRef =
    FIndexedColumnDefinition<TSharedRef<FOnlinePartyMemberRedpointEOS>>(
        FName(TEXT("OnlinePartyMemberRedpointEOSRef")),
        [](const TSharedRef<FOnlinePartyMemberRedpointEOS> &A, const TSharedRef<FOnlinePartyMemberRedpointEOS> &B) {
            return A == B;
        },
        [](const TSharedRef<FOnlinePartyMemberRedpointEOS> &K) {
            return GetTypeHash(K);
        });

TSharedRef<Features::FPartyConfigurationRoomFeatureRequest> FOnlinePartySystemRedpointEOS::
    ConvertPartyConfigurationToFeatureRequest(const FPartyConfiguration &PartyConfig) const
{
    Features::EPartyConfigurationRoomFeatureJoinRequestAction JoinRequestAction;
    Features::EPartyConfigurationRoomFeaturePermissionType JoinByPresencePermissions, SendInvitePermissions;

    switch (PartyConfig.JoinRequestAction)
    {
    case EJoinRequestAction::Manual:
        JoinRequestAction = Features::EPartyConfigurationRoomFeatureJoinRequestAction::Manual;
        break;
    case EJoinRequestAction::AutoApprove:
        JoinRequestAction = Features::EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove;
        break;
    case EJoinRequestAction::AutoReject:
        JoinRequestAction = Features::EPartyConfigurationRoomFeatureJoinRequestAction::AutoReject;
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Warning,
            TEXT("EJoinRequestAction::AutoReject is not supported by Epic Online Services and will behave as "
                 "EJoinRequestAction::AutoApprove."));
        break;
    default:
        checkf(false, TEXT("Unhandled EJoinRequestAction value!"));
        break;
    }

    switch (PartyConfig.PresencePermissions)
    {
    case PartySystemPermissions::EPermissionType::Noone:
        JoinByPresencePermissions = Features::EPartyConfigurationRoomFeaturePermissionType::Noone;
        break;
    case PartySystemPermissions::EPermissionType::Leader:
        JoinByPresencePermissions = Features::EPartyConfigurationRoomFeaturePermissionType::Leader;
        break;
    case PartySystemPermissions::EPermissionType::Friends:
        JoinByPresencePermissions = Features::EPartyConfigurationRoomFeaturePermissionType::Friends;
        break;
    case PartySystemPermissions::EPermissionType::Anyone:
        JoinByPresencePermissions = Features::EPartyConfigurationRoomFeaturePermissionType::Anyone;
        break;
    default:
        checkf(false, TEXT("Unhandled PartySystemPermissions::EPermissionType value!"));
        break;
    }

    switch (PartyConfig.InvitePermissions)
    {
    case PartySystemPermissions::EPermissionType::Noone:
        SendInvitePermissions = Features::EPartyConfigurationRoomFeaturePermissionType::Noone;
        break;
    case PartySystemPermissions::EPermissionType::Leader:
        SendInvitePermissions = Features::EPartyConfigurationRoomFeaturePermissionType::Leader;
        break;
    case PartySystemPermissions::EPermissionType::Friends:
        SendInvitePermissions = Features::EPartyConfigurationRoomFeaturePermissionType::Friends;
        break;
    case PartySystemPermissions::EPermissionType::Anyone:
        SendInvitePermissions = Features::EPartyConfigurationRoomFeaturePermissionType::Anyone;
        break;
    default:
        checkf(false, TEXT("Unhandled PartySystemPermissions::EPermissionType value!"));
        break;
    }

    return MakeShared<Features::FPartyConfigurationRoomFeatureRequest>(
        JoinRequestAction,
        JoinByPresencePermissions,
        SendInvitePermissions,
        PartyConfig.Nickname,
        PartyConfig.Description,
        PartyConfig.Password);
}

FRoomPtr FOnlinePartySystemRedpointEOS::GetUserCurrentRoom(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId) const
{
    auto PartyIdEOS = StaticCastSharedRef<const FOnlinePartyIdRedpointEOS>(PartyId.AsShared());
    auto CurrentRooms = this->RoomSystem->GetCurrentRooms(
        Requests::FGetCurrentRoomsRequest(ERoomScope::User, GetAccountId(LocalUserId), NAME_Game));
    auto *CurrentRoomPtr = CurrentRooms.FindByPredicate([PartyIdEOS](const FRoomRef &Value) {
        return Value->GetRoomId()->IsEqual(*PartyIdEOS->GetRoomId());
    });
    auto CurrentRoom = CurrentRoomPtr == nullptr ? FRoomPtr() : *CurrentRoomPtr;
    if (!CurrentRoom.IsValid() || !CurrentRoom->GetRoomFeature<Features::IPartyTypeRoomFeature>().IsValid())
    {
        return nullptr;
    }
    return CurrentRoom;
}

ECreatePartyCompletionResult FOnlinePartySystemRedpointEOS::ConvertErrorToCreatePartyCompletionResult(
    FError ErrorCode) const
{
    switch (ErrorCode.ResultCode())
    {
    case EOS_EResult::EOS_Lobby_SessionInProgress:
        return ECreatePartyCompletionResult::AlreadyInParty;
    case EOS_EResult::EOS_Lobby_VoiceNotEnabled:
        return ECreatePartyCompletionResult::AlreadyInPartyOfSpecifiedType;
    case EOS_EResult::EOS_RTC_RoomAlreadyExists:
    case EOS_EResult::EOS_RTC_UserKicked:
    case EOS_EResult::EOS_RTC_UserBanned:
    case EOS_EResult::EOS_RTC_RoomWasLeft:
    case EOS_EResult::EOS_RTC_ReconnectionTimegateExpired:
    case EOS_EResult::EOS_RTC_ShutdownInvoked:
        return ECreatePartyCompletionResult::FailedToCreateMucRoom;
    case EOS_EResult::EOS_MissingPermissions:
    case EOS_EResult::EOS_Missing_Feature:
        return ECreatePartyCompletionResult::LoggedOut;
    default:
        return ECreatePartyCompletionResult::UnknownClientFailure;
    }
}

EUpdateConfigCompletionResult FOnlinePartySystemRedpointEOS::ConvertErrorToUpdateConfigCompletionResult(
    FError ErrorCode) const
{
    switch (ErrorCode.ResultCode())
    {
    case EOS_EResult::EOS_Lobby_InvalidLock:
    case EOS_EResult::EOS_Lobby_InvalidSession:
    case EOS_EResult::EOS_Missing_Feature:
        return EUpdateConfigCompletionResult::LocalMemberNotLeader;
    case EOS_EResult::EOS_InvalidParameters:
        return EUpdateConfigCompletionResult::MessagingFailure;
    case EOS_EResult::EOS_Lobby_SandboxNotAllowed:
        return EUpdateConfigCompletionResult::UnknownParty;
    default:
        return EUpdateConfigCompletionResult::UnknownClientFailure;
    }
}

ELeavePartyCompletionResult FOnlinePartySystemRedpointEOS::ConvertErrorToLeavePartyCompletionResult(
    FError ErrorCode) const
{
    switch (ErrorCode.ResultCode())
    {
    case EOS_EResult::EOS_MissingPermissions:
    case EOS_EResult::EOS_Missing_Feature:
        return ELeavePartyCompletionResult::LoggedOut;
    case EOS_EResult::EOS_Lobby_SandboxNotAllowed:
        return ELeavePartyCompletionResult::UnknownParty;
    default:
        return ELeavePartyCompletionResult::UnknownClientFailure;
    }
}

ESendPartyInvitationCompletionResult FOnlinePartySystemRedpointEOS::ConvertErrorToSendPartyInvitationCompletionResult(
    FError ErrorCode) const
{
    switch (ErrorCode.ResultCode())
    {
    case EOS_EResult::EOS_Lobby_InvalidSession:
    case EOS_EResult::EOS_Lobby_MemberUpdateOnly:
        return ESendPartyInvitationCompletionResult::NoPermission;
    case EOS_EResult::EOS_Missing_Feature:
        return ESendPartyInvitationCompletionResult::NotLoggedIn;
    case EOS_EResult::EOS_Lobby_NoPermission:
        return ESendPartyInvitationCompletionResult::PartyFull;
    case EOS_EResult::EOS_AlreadyPending:
    case EOS_EResult::EOS_Disabled:
        return ESendPartyInvitationCompletionResult::RateLimited;
    default:
        return ESendPartyInvitationCompletionResult::UnknownInternalFailure;
    }
}

EKickMemberCompletionResult FOnlinePartySystemRedpointEOS::ConvertErrorToKickMemberCompletionResult(
    FError ErrorCode) const
{
    return EKickMemberCompletionResult::UnknownInternalFailure;
}

EPromoteMemberCompletionResult FOnlinePartySystemRedpointEOS::ConvertErrorToPromoteMemberCompletionResult(
    FError ErrorCode) const
{
    return EPromoteMemberCompletionResult::UnknownInternalFailure;
}

EJoinPartyCompletionResult FOnlinePartySystemRedpointEOS::ConvertErrorToJoinPartyCompletionResult(
    FError ErrorCode) const
{
    switch (ErrorCode.ResultCode())
    {
    case EOS_EResult::EOS_Lobby_TooManyPlayers:
    case EOS_EResult::EOS_Lobby_TooManyInvites:
        return EJoinPartyCompletionResult::NoSpace;
    case EOS_EResult::EOS_Lobby_InvalidSession:
    case EOS_EResult::EOS_Lobby_InviteFailed:
    case EOS_EResult::EOS_Lobby_InviteNotFound:
        return EJoinPartyCompletionResult::JoinInfoInvalid;
    case EOS_EResult::EOS_Lobby_LobbyAlreadyExists:
        return EJoinPartyCompletionResult::AlreadyInParty;
    case EOS_EResult::EOS_Lobby_NoPermission:
        return EJoinPartyCompletionResult::NotApproved;
    case EOS_EResult::EOS_Lobby_PlatformNotAllowed:
        return EJoinPartyCompletionResult::IncompatiblePlatform;
    case EOS_EResult::EOS_Missing_Feature:
        return EJoinPartyCompletionResult::IncompatiblePlatform;
    case EOS_EResult::EOS_Lobby_HostAtCapacity:
    case EOS_EResult::EOS_Lobby_AggregationFailed:
    case EOS_EResult::EOS_Lobby_DeploymentAtCapacity:
    case EOS_EResult::EOS_Lobby_InvalidLock:
        return EJoinPartyCompletionResult::UnknownInternalFailure;
    default:
        return EJoinPartyCompletionResult::UnknownClientFailure;
    }
}

void FOnlinePartySystemRedpointEOS::AddRoomToPartyDatabaseIfNotPresent(FRoomUserId LocalUserId, FRoomRef Room)
{
    FIndexedRowConstPtr IndexedRow = this->PartyDatabase.FindFirstByValue(CPartyDatabaseRoomRef.Create(Room));
    if (!IndexedRow.IsValid())
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("Room '%s' (0x%p) is being added to the PartyDatabase."),
            *Room->GetRoomId()->ToString(),
            &Room.Get());
        TSharedPtr<const FUniqueNetIdEOS> UserIdRedpointEOS = GetUniqueNetId(LocalUserId);
        checkf(UserIdRedpointEOS.IsValid(), TEXT("Expected user ID for room context to be valid!"));
        FUniqueNetIdAndPartyTypeId UserPartyTypeIdKey = FUniqueNetIdAndPartyTypeId(
            UserIdRedpointEOS.ToSharedRef(),
            FOnlinePartyTypeId(Room->GetRoomFeatureChecked<Features::IPartyTypeRoomFeature>()->GetPartyTypeId()));
        TSharedRef<const FOnlinePartyIdRedpointEOS> PartyIdRedpointEOS =
            MakeShared<FOnlinePartyIdRedpointEOS>(Room->GetRoomId());
        TSharedRef<FOnlinePartyRedpointEOS> PartyRedpointEOS =
            MakeSharedWithEvents<FOnlinePartyRedpointEOS>(this->RoomSystem, Room);
        FUniqueNetIdAndPartyId UserPartyIdKey =
            FUniqueNetIdAndPartyId(UserIdRedpointEOS.ToSharedRef(), PartyIdRedpointEOS);
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("User '%s' joined party '%s' which has party type '%u'."),
            *UserIdRedpointEOS->ToString(),
            *PartyIdRedpointEOS->ToString(),
            UserPartyTypeIdKey.Value.GetValue());
        for (const auto &RoomMember : Room->GetMembers())
        {
            FUniqueNetIdAndPartyIdAndMemberId UserPartyMemberIdKey = FUniqueNetIdAndPartyIdAndMemberId(
                UserIdRedpointEOS.ToSharedRef(),
                PartyIdRedpointEOS,
                GetUniqueNetId(RoomMember->GetUserId()).ToSharedRef());
            TSharedRef<FOnlinePartyMemberRedpointEOS> PartyMemberRedpointEOS =
                MakeSharedWithEvents<FOnlinePartyMemberRedpointEOS>(
                    this->UserFactory,
                    UserIdRedpointEOS.ToSharedRef(),
                    Room,
                    RoomMember,
                    RoomMember->GetUserId());
            this->PartyMemberDatabase.AddRow(
                {CPartyMemberDatabaseLocalUserIdAndPartyId.Create(UserPartyIdKey),
                 CPartyMemberDatabaseLocalUserIdAndPartyIdAndMemberId.Create(UserPartyMemberIdKey),
                 CPartyMemberDatabaseRoomMemberRef.Create(RoomMember),
                 CPartyMemberDatabaseOnlinePartyMemberRedpointEOSRef.Create(PartyMemberRedpointEOS)});
        }
        TSharedRef<FDelegateHandle> RoomMemberJoined = MakeShared<FDelegateHandle>(
            Room->OnRoomMemberJoined().Add(IRoom::FOnRoomMemberJoined::FDelegate::CreateSPLambda(
                this,
                [this, UserPartyIdKey, UserIdRedpointEOSRef = UserIdRedpointEOS.ToSharedRef(), PartyIdRedpointEOS](
                    FRoomEventContext RoomMemberEventContext,
                    FRoomRef RoomMemberRoom,
                    FRoomUserId RoomMemberId) {
                    if (GetAccountId(UserIdRedpointEOSRef) == RoomMemberEventContext.LocalUserId)
                    {
                        FUniqueNetIdAndPartyIdAndMemberId UserPartyMemberIdKey = FUniqueNetIdAndPartyIdAndMemberId(
                            UserIdRedpointEOSRef,
                            PartyIdRedpointEOS,
                            GetUniqueNetId(RoomMemberId).ToSharedRef());
                        FRoomMemberPtr RoomMemberPtr;
                        for (const auto &RoomMember : RoomMemberRoom->GetMembers())
                        {
                            if (RoomMember->GetUserId() == RoomMemberId)
                            {
                                RoomMemberPtr = RoomMember;
                                break;
                            }
                        }
                        checkf(RoomMemberPtr.IsValid(), TEXT("RoomMemberPtr must be valid!"));
                        TSharedRef<FOnlinePartyMemberRedpointEOS> PartyMemberRedpointEOS =
                            MakeSharedWithEvents<FOnlinePartyMemberRedpointEOS>(
                                this->UserFactory,
                                UserIdRedpointEOSRef,
                                RoomMemberRoom,
                                RoomMemberPtr.ToSharedRef(),
                                RoomMemberId);
                        this->PartyMemberDatabase.AddRow(
                            {CPartyMemberDatabaseLocalUserIdAndPartyId.Create(UserPartyIdKey),
                             CPartyMemberDatabaseLocalUserIdAndPartyIdAndMemberId.Create(UserPartyMemberIdKey),
                             CPartyMemberDatabaseRoomMemberRef.Create(RoomMemberPtr.ToSharedRef()),
                             CPartyMemberDatabaseOnlinePartyMemberRedpointEOSRef.Create(PartyMemberRedpointEOS)});
                        this->TriggerOnPartyMemberJoinedDelegates(
                            *GetUniqueNetId(RoomMemberEventContext.LocalUserId),
                            *PartyIdRedpointEOS,
                            *GetUniqueNetId(RoomMemberId));
                    }
                })));
        TSharedRef<FDelegateHandle> RoomMemberLeft = MakeShared<FDelegateHandle>(
            Room->OnRoomMemberLeft().Add(IRoom::FOnRoomMemberLeft::FDelegate::CreateSPLambda(
                this,
                [this, UserIdRedpointEOSRef = UserIdRedpointEOS.ToSharedRef(), PartyIdRedpointEOS](
                    FRoomEventContext RoomMemberEventContext,
                    FRoomRef RoomMemberRoom,
                    FRoomUserId RoomMemberId,
                    ERoomLeaveReason RoomLeaveReason) {
                    if (GetAccountId(UserIdRedpointEOSRef) == RoomMemberEventContext.LocalUserId)
                    {
                        FUniqueNetIdAndPartyIdAndMemberId UserPartyMemberIdKey = FUniqueNetIdAndPartyIdAndMemberId(
                            UserIdRedpointEOSRef,
                            PartyIdRedpointEOS,
                            GetUniqueNetId(RoomMemberId).ToSharedRef());
                        auto MemberIndexedRow = this->PartyMemberDatabase.FindFirstByValue(
                            CPartyMemberDatabaseLocalUserIdAndPartyIdAndMemberId.Create(UserPartyMemberIdKey));
                        this->PartyMemberDatabase.RemoveRow(MemberIndexedRow);
                        EMemberExitedReason ExitedReason = EMemberExitedReason::Left;
                        if (RoomLeaveReason == ERoomLeaveReason::RoomDestroyed)
                        {
                            ExitedReason = EMemberExitedReason::Removed;
                        }
                        else if (RoomLeaveReason == ERoomLeaveReason::KickedFromRoom)
                        {
                            ExitedReason = EMemberExitedReason::Kicked;
                        }
                        this->TriggerOnPartyMemberExitedDelegates(
                            *GetUniqueNetId(RoomMemberEventContext.LocalUserId),
                            *PartyIdRedpointEOS,
                            *GetUniqueNetId(RoomMemberId),
                            ExitedReason);
                    }
                })));
        TSharedRef<FDelegateHandle> RoomMemberUpdated = MakeShared<FDelegateHandle>(
            Room->OnRoomMemberUpdated().Add(IRoom::FOnRoomMemberUpdated::FDelegate::CreateSPLambda(
                this,
                [this, UserIdRedpointEOSRef = UserIdRedpointEOS.ToSharedRef(), PartyIdRedpointEOS](
                    FRoomEventContext RoomMemberEventContext,
                    FRoomRef RoomMemberRoom,
                    FRoomUserId RoomMemberId,
                    const FRoomMemberUpdateInfo &RoomMemberUpdateInfo) {
                    if (GetAccountId(UserIdRedpointEOSRef) == RoomMemberEventContext.LocalUserId)
                    {
                        FUniqueNetIdAndPartyIdAndMemberId UserPartyMemberIdKey = FUniqueNetIdAndPartyIdAndMemberId(
                            UserIdRedpointEOSRef,
                            PartyIdRedpointEOS,
                            GetUniqueNetId(RoomMemberId).ToSharedRef());
                        auto MemberIndexedRow = this->PartyMemberDatabase.FindFirstByValue(
                            CPartyMemberDatabaseLocalUserIdAndPartyIdAndMemberId.Create(UserPartyMemberIdKey));
                        if (MemberIndexedRow.IsValid())
                        {
                            if ((RoomMemberUpdateInfo.AttributesAdded.Num() > 0 ||
                                 RoomMemberUpdateInfo.AttributesUpdated.Num() > 0 ||
                                 RoomMemberUpdateInfo.AttributesDeleted.Num() > 0))
                            {
                                this->TriggerOnPartyMemberDataReceivedDelegates(
                                    *GetUniqueNetId(RoomMemberEventContext.LocalUserId),
                                    *PartyIdRedpointEOS,
                                    *GetUniqueNetId(RoomMemberId),
                                    DefaultPartyDataNamespace,
                                    *this->GetPartyMemberData(
                                        *GetUniqueNetId(RoomMemberEventContext.LocalUserId),
                                        *PartyIdRedpointEOS,
                                        *GetUniqueNetId(RoomMemberId),
                                        DefaultPartyDataNamespace));
                            }
                            else if (RoomMemberUpdateInfo.bInfoUpdated)
                            {
                                // @note: There is no event for non-data party member updates.
                            }
                        }
                        else
                        {
                            UE_LOG(
                                LogOnlineSubsystemRedpointEOS,
                                Error,
                                TEXT("Received OnRoomMemberUpdated for target user '%s', but local user '%s' in party "
                                     "'%s' didn't have a CPartyMemberDatabaseLocalUserIdAndPartyIdAndMemberId row to "
                                     "propagate the event."),
                                *GetUserIdString(RoomMemberId),
                                *GetUserIdString(RoomMemberEventContext.LocalUserId),
                                *PartyIdRedpointEOS->ToString());
                        }
                    }
                })));
        TSharedRef<FDelegateHandle> RoomMemberPromoted = MakeShared<FDelegateHandle>(
            Room->OnRoomMemberPromoted().Add(IRoom::FOnRoomMemberPromoted::FDelegate::CreateSPLambda(
                this,
                [this, UserIdRedpointEOSRef = UserIdRedpointEOS.ToSharedRef(), PartyIdRedpointEOS](
                    FRoomEventContext RoomMemberEventContext,
                    FRoomRef RoomMemberRoom,
                    FRoomUserId RoomMemberId) {
                    if (GetAccountId(UserIdRedpointEOSRef) == RoomMemberEventContext.LocalUserId)
                    {
                        FUniqueNetIdAndPartyIdAndMemberId UserPartyMemberIdKey = FUniqueNetIdAndPartyIdAndMemberId(
                            UserIdRedpointEOSRef,
                            PartyIdRedpointEOS,
                            GetUniqueNetId(RoomMemberId).ToSharedRef());
                        auto MemberIndexedRow = this->PartyMemberDatabase.FindFirstByValue(
                            CPartyMemberDatabaseLocalUserIdAndPartyIdAndMemberId.Create(UserPartyMemberIdKey));
                        if (MemberIndexedRow.IsValid())
                        {
                            this->TriggerOnPartyMemberPromotedDelegates(
                                *GetUniqueNetId(RoomMemberEventContext.LocalUserId),
                                *PartyIdRedpointEOS,
                                *GetUniqueNetId(RoomMemberId));
                        }
                    }
                })));
        this->PartyDatabase.AddRow(
            {CPartyDatabaseLocalUserId.Create(UserIdRedpointEOS.ToSharedRef()),
             CPartyDatabaseLocalUserIdAndPartyId.Create(UserPartyIdKey),
             CPartyDatabaseLocalUserIdAndPartyTypeId.Create(UserPartyTypeIdKey),
             CPartyDatabaseRoomRef.Create(Room),
             CPartyDatabaseOnlinePartyRedpointEOSRef.Create(PartyRedpointEOS),
             CPartyDatabasePartyMemberJoinedHandle.Create(RoomMemberJoined),
             CPartyDatabasePartyMemberLeftHandle.Create(RoomMemberLeft),
             CPartyDatabasePartyMemberUpdatedHandle.Create(RoomMemberUpdated),
             CPartyDatabasePartyMemberPromotedHandle.Create(RoomMemberPromoted)});
        this->TriggerOnPartyJoinedDelegates(*UserIdRedpointEOS.ToSharedRef(), *PartyIdRedpointEOS);
    }
    else
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("Room '%s' (0x%p) has already been added to PartyDatabase; we are not re-adding it."),
            *Room->GetRoomId()->ToString(),
            &Room.Get());
    }
}

void FOnlinePartySystemRedpointEOS::RemoveRoomFromPartyDatabaseIfPresent(FRoomUserId LocalUserId, FRoomRef Room)
{
    FIndexedRowConstPtr IndexedRow = this->PartyDatabase.FindFirstByValue(CPartyDatabaseRoomRef.Create(Room));
    if (IndexedRow.IsValid())
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("Room '%s' (0x%p) is being removed from the PartyDatabase."),
            *Room->GetRoomId()->ToString(),
            &Room.Get());
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("User '%s' left party '%s' which has party type '%u'."),
            *GetUserIdString(Room->GetBoundForLocalUser()),
            *Room->GetRoomId()->GetRoomId(),
            Room->GetRoomFeatureChecked<Features::IPartyTypeRoomFeature>()->GetPartyTypeId());
        auto RoomMemberJoined = CPartyDatabasePartyMemberJoinedHandle.Get(IndexedRow.ToSharedRef());
        auto RoomMemberLeft = CPartyDatabasePartyMemberLeftHandle.Get(IndexedRow.ToSharedRef());
        auto RoomMemberUpdated = CPartyDatabasePartyMemberUpdatedHandle.Get(IndexedRow.ToSharedRef());
        auto RoomMemberPromoted = CPartyDatabasePartyMemberPromotedHandle.Get(IndexedRow.ToSharedRef());
        Room->OnRoomMemberJoined().Remove(*RoomMemberJoined);
        Room->OnRoomMemberLeft().Remove(*RoomMemberLeft);
        Room->OnRoomMemberUpdated().Remove(*RoomMemberUpdated);
        Room->OnRoomMemberPromoted().Remove(*RoomMemberPromoted);
        this->TriggerOnPartyExitedDelegates(
            *CPartyDatabaseLocalUserIdAndPartyId.Get(IndexedRow.ToSharedRef()).Key,
            *CPartyDatabaseLocalUserIdAndPartyId.Get(IndexedRow.ToSharedRef()).Value);
        {
            TSharedPtr<const FUniqueNetIdEOS> UserIdRedpointEOS = GetUniqueNetId(LocalUserId);
            checkf(UserIdRedpointEOS.IsValid(), TEXT("Expected user ID for room context to be valid!"));
            TSharedRef<const FOnlinePartyIdRedpointEOS> PartyIdRedpointEOS =
                MakeShared<FOnlinePartyIdRedpointEOS>(Room->GetRoomId());
            FUniqueNetIdAndPartyId UserPartyIdKey =
                FUniqueNetIdAndPartyId(UserIdRedpointEOS.ToSharedRef(), PartyIdRedpointEOS);
            for (const auto &Row : this->PartyMemberDatabase.FindByValue(
                     CPartyMemberDatabaseLocalUserIdAndPartyId.Create(UserPartyIdKey)))
            {
                this->PartyMemberDatabase.RemoveRow(Row);
            }
        }
        this->PartyDatabase.RemoveRow(IndexedRow);
    }
    else
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("Room '%s' (0x%p) is not being removed from the PartyDatabase; it does not exist."),
            *Room->GetRoomId()->ToString(),
            &Room.Get());
    }
}

void FOnlinePartySystemRedpointEOS::JoinPartyOnRequest(
    const FRoomEventContext &EventContext,
    const FRoomRequestedJoinRef &RoomRequestedJoin)
{
    // Initialize the join request.
    auto JoinRequest = Requests::FJoinRoomRequest(
        ERoomScope::User,
        EventContext.LocalUserId,
        RoomRequestedJoin->GetRoomSearchResult());

    // If the presence room feature is enabled on the search result, then we should enable presence as well.
    auto PresenceSearchState = RoomRequestedJoin->GetRoomSearchResult()
                                   ->GetRoomFeatureSearchState<Features::IPresenceRoomFeatureSearchState>();
    if (PresenceSearchState.IsValid())
    {
        JoinRequest.AddFeatureRequest(
            MakeShared<Features::FPresenceRoomFeatureRequest>(PresenceSearchState->AllowJoinFromPresence()));
    }

    // Execute the join room operation.
    this->RoomSystem->JoinRoom(
        JoinRequest,
        IRoomSystem::FOnJoinRoomComplete::CreateSPLambda(this, [this, EventContext](FError ErrorCode, FRoomPtr Room) {
            if (!ErrorCode.WasSuccessful())
            {
                UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
            }
            else
            {
                checkf(Room.IsValid(), TEXT("Expected Room to be valid when ErrorCode was successful!"));
                this->AddRoomToPartyDatabaseIfNotPresent(EventContext.LocalUserId, Room.ToSharedRef());
            }
        }));
}

void FOnlinePartySystemRedpointEOS::LeavePartyOnRequest(
    const FRoomEventContext &EventContext,
    const FRoomRequestedLeaveRef &RoomRequestedLeave)
{
    // Initialize and execute the leave room request.
    auto LeaveRequest = Requests::FLeaveRoomRequest();
    this->RoomSystem->LeaveRoom(
        RoomRequestedLeave->GetRoom(),
        LeaveRequest,
        IRoomSystem::FOnLeaveRoomComplete::CreateSPLambda(
            this,
            [this, EventContext, RoomRequestedLeave](FError ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                }
                else
                {
                    this->RemoveRoomFromPartyDatabaseIfPresent(EventContext.LocalUserId, RoomRequestedLeave->GetRoom());
                }
            }));
}

FOnlinePartySystemRedpointEOS::FOnlinePartySystemRedpointEOS(
    const FPlatformHandle &InPlatformHandle,
    const TSharedRef<IRoomSystem> &InRoomSystem,
    const TSharedRef<FEOSUserFactory> &InUserFactory,
    const TSharedRef<FEOSVoiceManager> &InVoiceManager,
    const TSharedRef<FOnlineSocialInterfaceEOS> &InSocialImpl)
    : PlatformHandle(InPlatformHandle)
    , RoomSystem(InRoomSystem)
    , UserFactory(InUserFactory)
    , VoiceManager(InVoiceManager)
    , Social(InSocialImpl)
    , PartyDatabase()
    , PartyMemberDatabase()
    , RequestToJoins()
    , OnRoomJoinedHandle()
    , OnRoomUpdatedHandle()
    , OnRoomLeftHandle()
    , OnRoomInviteReceivedHandle()
    , OnRoomInviteRemovedHandle()
    , OnRoomJoinRequestedHandle()
    , OnRoomLeaveRequestedHandle()
{
    PartyDatabase.AddIndexedColumn(CPartyDatabaseLocalUserId)
        .AddIndexedColumn(CPartyDatabaseLocalUserIdAndPartyId, true)
        .AddIndexedColumn(CPartyDatabaseLocalUserIdAndPartyTypeId, true)
        .AddIndexedColumn(CPartyDatabaseRoomRef, true)
        .AddIndexedColumn(CPartyDatabaseOnlinePartyRedpointEOSRef, true)
        .AddIndexedColumn(CPartyDatabasePartyMemberJoinedHandle, true)
        .AddIndexedColumn(CPartyDatabasePartyMemberLeftHandle, true)
        .AddIndexedColumn(CPartyDatabasePartyMemberUpdatedHandle, true)
        .AddIndexedColumn(CPartyDatabasePartyMemberPromotedHandle, true);
    PartyMemberDatabase.AddIndexedColumn(CPartyMemberDatabaseLocalUserIdAndPartyId)
        .AddIndexedColumn(CPartyMemberDatabaseLocalUserIdAndPartyIdAndMemberId, true)
        .AddIndexedColumn(CPartyMemberDatabaseRoomMemberRef, true)
        .AddIndexedColumn(CPartyMemberDatabaseOnlinePartyMemberRedpointEOSRef, true);
}

void FOnlinePartySystemRedpointEOS::RegisterEvents()
{
    this->OnRoomJoinedHandle =
        this->RoomSystem->OnRoomJoined(NAME_Game).Add(IRoomSystem::FOnRoomJoined::FDelegate::CreateSPLambda(
            this,
            [this](FRoomEventContext RoomEventContext, FRoomRef Room, ERoomJoinReason RoomJoinReason) {
                if (RoomEventContext.RoomScope == ERoomScope::User &&
                    Room->HasRoomFeature<Features::IPartyTypeRoomFeature>() &&
                    !this->PartyDatabase.FindFirstByValue(CPartyDatabaseRoomRef.Create(Room)).IsValid())
                {
                    this->AddRoomToPartyDatabaseIfNotPresent(RoomEventContext.LocalUserId, Room);
                    this->VoiceManager->UserManuallyJoinedLobby(
                        *GetUniqueNetId(RoomEventContext.LocalUserId),
                        *FOnlinePartyIdRedpointBase::Create(Room->GetRoomId()));
                }
            }));
    this->OnRoomUpdatedHandle =
        this->RoomSystem->OnRoomUpdated(NAME_Game).Add(IRoomSystem::FOnRoomUpdated::FDelegate::CreateSPLambda(
            this,
            [this](FRoomEventContext RoomEventContext, FRoomRef Room, const FRoomUpdateInfo &RoomUpdateInfo) {
                if (RoomEventContext.RoomScope == ERoomScope::User &&
                    Room->HasRoomFeature<Features::IPartyTypeRoomFeature>() &&
                    this->PartyDatabase.FindFirstByValue(CPartyDatabaseRoomRef.Create(Room)).IsValid())
                {
                    auto UserIdRedpointEOS = GetUniqueNetId(RoomEventContext.LocalUserId);
                    auto PartyIdRedpointEOS = MakeShared<FOnlinePartyIdRedpointEOS>(Room->GetRoomId());
                    if ((RoomUpdateInfo.AttributesAdded.Num() > 0 || RoomUpdateInfo.AttributesUpdated.Num() > 0 ||
                         RoomUpdateInfo.AttributesDeleted.Num() > 0))
                    {
                        UE_LOG(
                            LogOnlineSubsystemRedpointEOS,
                            Verbose,
                            TEXT("User '%s' saw party data updated for '%s'."),
                            *UserIdRedpointEOS->ToString(),
                            *PartyIdRedpointEOS->ToString());
                        this->TriggerOnPartyDataReceivedDelegates(
                            *UserIdRedpointEOS,
                            *PartyIdRedpointEOS,
                            DefaultPartyDataNamespace,
                            *this->GetPartyData(*UserIdRedpointEOS, *PartyIdRedpointEOS, DefaultPartyDataNamespace));
                    }
                    else if (
                        RoomUpdateInfo.FeaturesUpdated.Contains(
                            Features::FPartyConfigurationRoomFeatureRequest::GetFeatureNameStatic()) ||
                        RoomUpdateInfo.bMaxMembersChanged ||
                        RoomUpdateInfo.FeaturesUpdated.Contains(
                            Features::FInvitesRoomFeatureRequest::GetFeatureNameStatic()) ||
                        RoomUpdateInfo.FeaturesUpdated.Contains(
                            Features::FPresenceRoomFeatureRequest::GetFeatureNameStatic()) ||
                        RoomUpdateInfo.FeaturesUpdated.Contains(
                            Features::FPublicAdvertisementRoomFeatureRequest::GetFeatureNameStatic()))
                    {
                        UE_LOG(
                            LogOnlineSubsystemRedpointEOS,
                            Verbose,
                            TEXT("User '%s' saw party config changed for '%s'."),
                            *UserIdRedpointEOS->ToString(),
                            *PartyIdRedpointEOS->ToString());
                        this->TriggerOnPartyConfigChangedDelegates(
                            *UserIdRedpointEOS,
                            *PartyIdRedpointEOS,
                            *this->GetParty(*UserIdRedpointEOS, *PartyIdRedpointEOS)->GetConfiguration());
                    }
                }
            }));
    this->OnRoomLeftHandle =
        this->RoomSystem->OnRoomLeft(NAME_Game).Add(IRoomSystem::FOnRoomLeft::FDelegate::CreateSPLambda(
            this,
            [this](FRoomEventContext RoomEventContext, FRoomRef Room, ERoomLeaveReason RoomLeaveReason) {
                this->VoiceManager->UserManuallyLeftLobby(
                    *GetUniqueNetId(RoomEventContext.LocalUserId),
                    *FOnlinePartyIdRedpointBase::Create(Room->GetRoomId()));
                this->RemoveRoomFromPartyDatabaseIfPresent(RoomEventContext.LocalUserId, Room);
            }));
    this->OnRoomInviteReceivedHandle = this->RoomSystem->OnRoomInviteReceived(NAME_Game).Add(
        IRoomSystem::FOnRoomInviteReceived::FDelegate::CreateSPLambda(
            this,
            [this](FRoomEventContext RoomEventContext, FRoomInviteRef RoomInvite) {
                if (RoomInvite->GetRoomInviteSearchResult()
                        ->HasRoomFeatureSearchState<Features::IPartyTypeRoomFeatureSearchState>())
                {
                    TSharedPtr<const FUniqueNetIdEOS> UserIdRedpointEOS = GetUniqueNetId(RoomEventContext.LocalUserId);
                    TSharedRef<IOnlinePartyJoinInfo> JoinInfo =
                        MakeShared<FOnlinePartyJoinInfoRedpointEOSRoomInvite>(this->PlatformHandle, RoomInvite);
                    this->TriggerOnPartyInviteReceivedExDelegates(*UserIdRedpointEOS, *JoinInfo);
                    this->TriggerOnPartyInvitesChangedDelegates(*UserIdRedpointEOS);
                }
            }));
    this->OnRoomInviteRemovedHandle = this->RoomSystem->OnRoomInviteRemoved(NAME_Game).Add(
        IRoomSystem::FOnRoomInviteRemoved::FDelegate::CreateSPLambda(
            this,
            [this](
                FRoomEventContext RoomEventContext,
                FRoomInviteRef RoomInvite,
                IRoomSystem::ERoomInviteRemovedReason RoomInviteRemovedReason) {
                if (RoomInvite->GetRoomInviteSearchResult()
                        ->HasRoomFeatureSearchState<Features::IPartyTypeRoomFeatureSearchState>())
                {
                    // @todo: Do we need to guarantee FOnlinePartyJoinInfoRedpointEOSRoomInvite is the same instance as
                    // when it was received? We'd also then need to guarantee it for GetPendingInvites.
                    TSharedPtr<const FUniqueNetIdEOS> UserIdRedpointEOS = GetUniqueNetId(RoomEventContext.LocalUserId);
                    TSharedRef<IOnlinePartyJoinInfo> JoinInfo =
                        MakeShared<FOnlinePartyJoinInfoRedpointEOSRoomInvite>(this->PlatformHandle, RoomInvite);
                    switch (RoomInviteRemovedReason)
                    {
                    case IRoomSystem::ERoomInviteRemovedReason::Accepted:
                        this->TriggerOnPartyInviteRemovedExDelegates(
                            *UserIdRedpointEOS,
                            *JoinInfo,
                            EPartyInvitationRemovedReason::Accepted);
                        this->TriggerOnPartyInvitesChangedDelegates(*UserIdRedpointEOS);
                        break;
                    case IRoomSystem::ERoomInviteRemovedReason::Rejected:
                        this->TriggerOnPartyInviteRemovedExDelegates(
                            *UserIdRedpointEOS,
                            *JoinInfo,
                            EPartyInvitationRemovedReason::Declined);
                        this->TriggerOnPartyInvitesChangedDelegates(*UserIdRedpointEOS);
                        break;
                    default:
                        checkf(false, TEXT("Unhandled IRoomSystem::ERoomInviteRemovedReason value!"));
                        break;
                    }
                }
            }));
    this->OnRoomJoinRequestedHandle = this->RoomSystem->OnRoomJoinRequested(NAME_Game).Add(
        IRoomSystem::FOnRoomJoinRequested::FDelegate::CreateSPLambda(
            this,
            [this](const FRoomEventContext &EventContext, const FRoomRequestedJoinRef &RoomRequestedJoin) {
                if (RoomRequestedJoin->GetRoomSearchResult()
                        ->HasRoomFeatureSearchState<Features::IPartyTypeRoomFeatureSearchState>())
                {
                    bool bAutoApprove = true;
                    auto ConfigurationSearchState =
                        RoomRequestedJoin->GetRoomSearchResult()
                            ->GetRoomFeatureSearchState<Features::IPartyConfigurationRoomFeatureSearchState>();
                    if (ConfigurationSearchState.IsValid())
                    {
                        bAutoApprove = ConfigurationSearchState->GetJoinRequestAction() !=
                                       Features::EPartyConfigurationRoomFeatureJoinRequestAction::Manual;
                    }

                    if (bAutoApprove)
                    {
                        UE_LOG(
                            LogOnlineSubsystemRedpointEOS,
                            Verbose,
                            TEXT("Request to join party from overlay or runtime platform is being being automatically "
                                 "handled because 'JoinRequestAction' is set to 'AutoApprove' (or 'AutoReject' which "
                                 "is not supported)."));
                        this->JoinPartyOnRequest(EventContext, RoomRequestedJoin);
                    }
                    else
                    {
                        UE_LOG(
                            LogOnlineSubsystemRedpointEOS,
                            Verbose,
                            TEXT("Triggering the 'OnPartyRequestToJoin' event for a 'request to join party' from "
                                 "overlay or runtime platform as the 'JoinRequestAction' on the party is set to "
                                 "'Manual'."));
                        auto JoinRequest =
                            MakeShared<FOnlinePartyRequestToJoinInfoRedpointEOS>(EventContext, RoomRequestedJoin);
                        this->RequestToJoins.FindOrAdd(JoinRequest->GetUserId().ToSharedRef()).Add(JoinRequest);
                        this->TriggerOnPartyRequestToJoinReceivedDelegates(
                            *JoinRequest->GetUserId(),
                            *JoinRequest->GetPartyId(),
                            *JoinRequest->GetUserId(),
                            *JoinRequest);
                    }
                }
            }));
    this->OnRoomLeaveRequestedHandle = this->RoomSystem->OnRoomLeaveRequested(NAME_Game).Add(
        IRoomSystem::FOnRoomLeaveRequested::FDelegate::CreateSPLambda(
            this,
            [this](const FRoomEventContext &EventContext, const FRoomRequestedLeaveRef &RoomRequestedLeave) {
                if (RoomRequestedLeave->GetRoom()->HasRoomFeature<Features::IPartyTypeRoomFeature>())
                {
                    this->LeavePartyOnRequest(EventContext, RoomRequestedLeave);
                }
            }));
}

FOnlinePartySystemRedpointEOS::~FOnlinePartySystemRedpointEOS()
{
    this->RoomSystem->OnRoomJoined(NAME_Game).Remove(this->OnRoomJoinedHandle);
    this->RoomSystem->OnRoomUpdated(NAME_Game).Remove(this->OnRoomUpdatedHandle);
    this->RoomSystem->OnRoomLeft(NAME_Game).Remove(this->OnRoomLeftHandle);
    this->RoomSystem->OnRoomInviteReceived(NAME_Game).Remove(this->OnRoomInviteReceivedHandle);
    this->RoomSystem->OnRoomInviteRemoved(NAME_Game).Remove(this->OnRoomInviteRemovedHandle);
    this->RoomSystem->OnRoomJoinRequested(NAME_Game).Remove(this->OnRoomJoinRequestedHandle);
    this->RoomSystem->OnRoomLeaveRequested(NAME_Game).Remove(this->OnRoomLeaveRequestedHandle);
    // @note: Intentional copy of the set so we can remove items.
    for (const auto &IndexedRow : TSet<FIndexedRowConstRef>(this->PartyDatabase.GetAllRows()))
    {
        auto Room = CPartyDatabaseRoomRef.Get(IndexedRow);
        auto RoomMemberJoined = CPartyDatabasePartyMemberJoinedHandle.Get(IndexedRow);
        auto RoomMemberLeft = CPartyDatabasePartyMemberLeftHandle.Get(IndexedRow);
        Room->OnRoomMemberJoined().Remove(*RoomMemberJoined);
        Room->OnRoomMemberLeft().Remove(*RoomMemberLeft);
        this->PartyDatabase.RemoveRow(IndexedRow);
    }
}

void FOnlinePartySystemRedpointEOS::RestoreParties(
    const FUniqueNetId &LocalUserId,
    const FOnRestorePartiesComplete &CompletionDelegate)
{
    CompletionDelegate.ExecuteIfBound(LocalUserId, AsOSSError(Errors::Success()));
}

void FOnlinePartySystemRedpointEOS::RestoreInvites(
    const FUniqueNetId &LocalUserId,
    const FOnRestoreInvitesComplete &CompletionDelegate)
{
    this->RoomSystem->RestoreRoomInvites(
        Requests::FRestoreRoomInvitesRequest{GetAccountId(LocalUserId)},
        IRoomSystem::FOnRestoreRoomInvitesComplete::CreateSPLambda(
            this,
            [CompletionDelegate, UserId = LocalUserId.AsShared()](FError ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                }
                CompletionDelegate.ExecuteIfBound(*UserId, AsOSSError(ErrorCode));
            }));
}

void FOnlinePartySystemRedpointEOS::CleanupParties(
    const FUniqueNetId &LocalUserId,
    const FOnCleanupPartiesComplete &CompletionDelegate)
{
    CompletionDelegate.ExecuteIfBound(LocalUserId, AsOSSError(Errors::Success()));
}

bool FOnlinePartySystemRedpointEOS::CreateParty(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyTypeId PartyTypeId,
    const FPartyConfiguration &PartyConfig,
    const FOnCreatePartyComplete &Delegate)
{
    // Initialize the room creation request based on the party configuration.
    auto CreateRequest = Requests::FCreateRoomRequest(
        ERoomScope::User,
        GetAccountId(LocalUserId),
        FName(TEXT("Lobby")),
        NAME_Game,
        static_cast<uint32>(PartyConfig.MaxMembers));

    // Ensure we set the bucket ID to isolate parties from other lobbies.
    CreateRequest.AddFeatureRequest(MakeShared<Features::FBucketRoomFeatureRequest>(DefaultPartyBucketId));

    // Store the party configuration and party type ID as attributes on the underlying lobby so that these values are
    // consistent for both the party leader and party members.
    CreateRequest.AddFeatureRequest(this->ConvertPartyConfigurationToFeatureRequest(PartyConfig));
    CreateRequest.AddFeatureRequest(MakeShared<Features::FPartyTypeRoomFeatureRequest>(PartyTypeId.GetValue()));

    // Turn on invites if requested.
    if (PartyConfig.bIsAcceptingMembers &&
        PartyConfig.InvitePermissions != PartySystemPermissions::EPermissionType::Noone)
    {
        if (PartyConfig.InvitePermissions == PartySystemPermissions::EPermissionType::Leader ||
            PartyConfig.InvitePermissions == PartySystemPermissions::EPermissionType::Friends)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Warning,
                TEXT("EPermissionType::Leader and EPermissionType::Friends are not supported values for "
                     "InvitePermissions. These will be treated as EPermissionType::Anyone."));
        }
        CreateRequest.AddFeatureRequest(MakeShared<Features::FInvitesRoomFeatureRequest>(true));
    }

    // Turn on join-by-ID so that synthetic parties can directly join regardless of invite or presence settings.
    CreateRequest.AddFeatureRequest(MakeShared<Features::FJoinByIdRoomFeatureRequest>());

    // Turn on public member lists so that when developers are querying for parties via the IOnlineLobby interface, they
    // can query the members that are currently in the party.
    CreateRequest.AddFeatureRequest(MakeShared<Features::FPublicMemberListRoomFeatureRequest>());

    // If we don't have "should remove on disconnection" enabled, turn on owner migration so that another member will be
    // promoted to party leader when the party leader leaves.
    if (!PartyConfig.bShouldRemoveOnDisconnection)
    {
        CreateRequest.AddFeatureRequest(MakeShared<Features::FOwnerMigrationRoomFeatureRequest>());
    }

    // Determine whether this party should have presence enabled (based on it's party type ID), and determine whether
    // we'll permit people to join via presence based on the PresencePermissions.
    bool bAllowJoinByPresence = false;
    if (PartyConfig.bIsAcceptingMembers)
    {
        switch (PartyConfig.PresencePermissions)
        {
        case PartySystemPermissions::EPermissionType::Noone:
            bAllowJoinByPresence = false;
            break;
        case PartySystemPermissions::EPermissionType::Leader:
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Warning,
                TEXT("EPermissionType::Leader is not a supported value for PresencePermissions and will be treated as "
                     "EPermissionType::Friends instead."));
            bAllowJoinByPresence = true;
            break;
        case PartySystemPermissions::EPermissionType::Friends:
        case PartySystemPermissions::EPermissionType::Anyone:
            bAllowJoinByPresence = true;
            break;
        default:
            checkf(false, TEXT("Unhandled PartySystemPermissions::EPermissionType value!"));
            break;
        }
    }
    if (PartyTypeId == this->GetPrimaryPartyTypeId())
    {
        CreateRequest.AddFeatureRequest(MakeShared<Features::FPresenceRoomFeatureRequest>(bAllowJoinByPresence));
    }
    else if (bAllowJoinByPresence)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Warning,
            TEXT("EPermissionType::Leader, EPermissionType::Friends and EPermissionType::Anyone are not supported "
                 "values for PresencePermissions when the party type ID is not the primary party ID type. Parties "
                 "that do not use the primary party type ID will not have presence enabled and therefore it is not "
                 "possible to join-via-presence for these parties."));
    }

    // If the presence permissions is set to Anyone, then turn on public advertisement so this party can be found by
    // anyone.
    if (PartyConfig.bIsAcceptingMembers &&
        PartyConfig.PresencePermissions == PartySystemPermissions::EPermissionType::Anyone)
    {
        CreateRequest.AddFeatureRequest(MakeShared<Features::FPublicAdvertisementRoomFeatureRequest>());
    }

    // Turn on voice chat if requested.
    if (PartyConfig.bChatEnabled)
    {
        auto VoiceChatFlags = RTC::EJoinRoomOptions::None;
#if !UE_BUILD_SHIPPING
        if (this->PlatformHandle->GetEnvironment()->Config->GetEnableVoiceChatEchoInParties())
        {
            VoiceChatFlags = RTC::EJoinRoomOptions::EnableEcho;
        }
#endif
        CreateRequest.AddFeatureRequest(
            MakeShared<Features::FVoiceChatRoomFeatureRequest>(VoiceChatFlags, false, false, false));
    }

    // Create the room for the party.
    this->RoomSystem->CreateRoom(
        CreateRequest,
        IRoomSystem::FOnCreateRoomComplete::CreateSPLambda(
            this,
            [this, Delegate, UserId = LocalUserId.AsShared()](FError ErrorCode, FRoomPtr Room) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                    Delegate.ExecuteIfBound(
                        *UserId,
                        nullptr,
                        this->ConvertErrorToCreatePartyCompletionResult(ErrorCode));
                }
                else
                {
                    checkf(Room.IsValid(), TEXT("Expected Room to be valid when ErrorCode was successful!"));
                    this->AddRoomToPartyDatabaseIfNotPresent(GetAccountId(UserId), Room.ToSharedRef());
                    Delegate.ExecuteIfBound(
                        *UserId,
                        MakeShared<FOnlinePartyIdRedpointEOS>(Room->GetRoomId()),
                        ECreatePartyCompletionResult::Succeeded);
                }
            }));
    return true;
}

bool FOnlinePartySystemRedpointEOS::UpdateParty(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FPartyConfiguration &PartyConfig,
    bool bShouldRegenerateReservationKey,
    const FOnUpdatePartyComplete &Delegate)
{
    // Try to find the target room, and make sure the local user is the owner.
    auto CurrentRoom = this->GetUserCurrentRoom(LocalUserId, PartyId);
    if (!CurrentRoom.IsValid())
    {
        Delegate.ExecuteIfBound(LocalUserId, PartyId, EUpdateConfigCompletionResult::UnknownParty);
        return true;
    }
    if (CurrentRoom->GetRoomOwner()->IsDedicatedServer() ||
        CurrentRoom->GetRoomOwner()->GetUserId() != GetAccountId(LocalUserId))
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("You can not update this party because you are not the leader of it."));
        Delegate.ExecuteIfBound(LocalUserId, PartyId, EUpdateConfigCompletionResult::LocalMemberNotLeader);
        return true;
    }

    // Initialize the room update request.
    auto UpdateRequest = Requests::FUpdateRoomRequest();

    // Update the maximum member count based on the party configuration.
    UpdateRequest.MaxMemberCount = static_cast<uint32>(PartyConfig.MaxMembers);

    // Synchronise the party configuration as attributes on the underlying lobby so that these values are consistent for
    // both the party leader and party members.
    UpdateRequest.AddFeatureRequest(this->ConvertPartyConfigurationToFeatureRequest(PartyConfig));

    // Turn invites on or off depending on the party configuration.
    if (PartyConfig.bIsAcceptingMembers &&
        PartyConfig.InvitePermissions != PartySystemPermissions::EPermissionType::Noone)
    {
        if (PartyConfig.InvitePermissions == PartySystemPermissions::EPermissionType::Leader ||
            PartyConfig.InvitePermissions == PartySystemPermissions::EPermissionType::Friends)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Warning,
                TEXT("EPermissionType::Leader and EPermissionType::Friends are not supported values for "
                     "InvitePermissions. These will be treated as EPermissionType::Anyone."));
        }
        UpdateRequest.AddFeatureRequest(MakeShared<Features::FInvitesRoomFeatureRequest>(true));
    }
    else
    {
        UpdateRequest.DeleteFeature(Features::FInvitesRoomFeatureRequest::GetFeatureNameStatic());
    }

    // Ensure that "should remove on disconnection" matches the current behaviour.
    bool bCurrentShouldRemoveOnDisconnection =
        !CurrentRoom->GetRoomFeature<Features::IOwnerMigrationRoomFeature>().IsValid();
    if (PartyConfig.bShouldRemoveOnDisconnection != bCurrentShouldRemoveOnDisconnection)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("Changing the value of bShouldRemoveOnDisconnection is not supported by the EOS SDK."));
        Delegate.ExecuteIfBound(LocalUserId, PartyId, EUpdateConfigCompletionResult::UnknownClientFailure);
        return true;
    }

    // Update the "join via presence" permission if this party has presence enabled, based on the PresencePermissions.
    if (CurrentRoom->GetRoomFeature<Features::IPresenceRoomFeature>().IsValid())
    {
        bool bAllowJoinByPresence = false;
        if (PartyConfig.bIsAcceptingMembers)
        {
            switch (PartyConfig.PresencePermissions)
            {
            case PartySystemPermissions::EPermissionType::Noone:
                bAllowJoinByPresence = false;
                break;
            case PartySystemPermissions::EPermissionType::Leader:
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    Warning,
                    TEXT("EPermissionType::Leader is not a supported value for PresencePermissions and will be treated "
                         "as EPermissionType::Friends instead."));
                bAllowJoinByPresence = true;
                break;
            case PartySystemPermissions::EPermissionType::Friends:
            case PartySystemPermissions::EPermissionType::Anyone:
                bAllowJoinByPresence = true;
                break;
            default:
                checkf(false, TEXT("Unhandled PartySystemPermissions::EPermissionType value!"));
                break;
            }
        }
        UpdateRequest.AddFeatureRequest(MakeShared<Features::FPresenceRoomFeatureRequest>(bAllowJoinByPresence));
    }

    // Update the "public advertisement" setting based on the PresencePermissions.
    if (PartyConfig.bIsAcceptingMembers &&
        PartyConfig.PresencePermissions == PartySystemPermissions::EPermissionType::Anyone)
    {
        UpdateRequest.AddFeatureRequest(MakeShared<Features::FPublicAdvertisementRoomFeatureRequest>());
    }
    else
    {
        UpdateRequest.DeleteFeature(Features::FPublicAdvertisementRoomFeatureRequest::GetFeatureNameStatic());
    }

    // Turn on voice chat if requested.
    bool bCurrentVoiceChatEnabled = CurrentRoom->GetRoomFeature<Features::IVoiceChatRoomFeature>().IsValid();
    if (PartyConfig.bChatEnabled != bCurrentVoiceChatEnabled)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("Changing the value of bChatEnabled is not supported by the EOS SDK."));
        Delegate.ExecuteIfBound(LocalUserId, PartyId, EUpdateConfigCompletionResult::UnknownClientFailure);
        return true;
    }

    // Update the room for the party.
    this->RoomSystem->UpdateRoom(
        CurrentRoom.ToSharedRef(),
        UpdateRequest,
        IRoomSystem::FOnUpdateRoomComplete::CreateSPLambda(
            this,
            [this, Delegate, UserId = LocalUserId.AsShared(), CurrentRoom](FError ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                    Delegate.ExecuteIfBound(
                        *UserId,
                        *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                        this->ConvertErrorToUpdateConfigCompletionResult(ErrorCode));
                }
                else
                {
                    Delegate.ExecuteIfBound(
                        *UserId,
                        *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                        EUpdateConfigCompletionResult::Succeeded);
                }
            }));
    return true;
}

bool FOnlinePartySystemRedpointEOS::JoinParty(
    const FUniqueNetId &LocalUserId,
    const IOnlinePartyJoinInfo &OnlinePartyJoinInfo,
    const FOnJoinPartyComplete &Delegate)
{
    // Initialize the room join request based on the party configuration.
    auto OnlinePartyJoinInfoEOS =
        StaticCastSharedRef<const FOnlinePartyJoinInfoRedpointEOS>(OnlinePartyJoinInfo.AsShared());
    auto JoinRequest = OnlinePartyJoinInfoEOS->GetJoinRoomRequest(LocalUserId);

    // Ensure the join request is for this local user.
    if (JoinRequest.LocalUser != GetAccountId(LocalUserId))
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s"),
            *Errors::InvalidRequest(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 FString::Printf(
                     TEXT("The OnlinePartyJoinInfo passed to JoinParty is for user %s, but user %s attempted to use "
                          "it."),
                     *GetUserIdString(JoinRequest.LocalUser),
                     *LocalUserId.ToString()))
                 .ToLogString());
        Delegate.ExecuteIfBound(
            LocalUserId,
            *MakeShared<FOnlinePartyIdRedpointEOS>(),
            EJoinPartyCompletionResult::JoinInfoInvalid,
            0);
        return true;
    }

    // If we know this is the primary party (which is the case when we get a "room ID" from GetAdvertisedParty), enable
    // presence as well.
    if (OnlinePartyJoinInfoEOS->IsKnownToBePrimaryParty())
    {
        // @note: We assume "allow join by presence" is allowed.
        JoinRequest.AddFeatureRequest(MakeShared<Features::FPresenceRoomFeatureRequest>(true));
    }
    else
    {
        // If the presence room feature is enabled on the search result, then we should enable presence as well.
        FRoomSearchResultPtr JoinRoomSearchResult = OnlinePartyJoinInfoEOS->GetRoomSearchResult();
        if (JoinRoomSearchResult.IsValid())
        {
            auto PresenceSearchState =
                JoinRoomSearchResult->GetRoomFeatureSearchState<Features::IPresenceRoomFeatureSearchState>();
            if (PresenceSearchState.IsValid())
            {
                JoinRequest.AddFeatureRequest(
                    MakeShared<Features::FPresenceRoomFeatureRequest>(PresenceSearchState->AllowJoinFromPresence()));
            }
        }
    }

    // Execute the join room operation.
    this->RoomSystem->JoinRoom(
        JoinRequest,
        IRoomSystem::FOnJoinRoomComplete::CreateSPLambda(
            this,
            [this, Delegate, UserId = LocalUserId.AsShared()](FError ErrorCode, FRoomPtr Room) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                    Delegate.ExecuteIfBound(
                        *UserId,
                        *MakeShared<FOnlinePartyIdRedpointEOS>(),
                        this->ConvertErrorToJoinPartyCompletionResult(ErrorCode),
                        0);
                }
                else
                {
                    checkf(Room.IsValid(), TEXT("Expected Room to be valid when ErrorCode was successful!"));
                    this->AddRoomToPartyDatabaseIfNotPresent(GetAccountId(UserId), Room.ToSharedRef());
                    Delegate.ExecuteIfBound(
                        *UserId,
                        *MakeShared<FOnlinePartyIdRedpointEOS>(Room->GetRoomId()),
                        EJoinPartyCompletionResult::Succeeded,
                        0);
                }
            }));
    return true;
}

void FOnlinePartySystemRedpointEOS::RequestToJoinParty(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyTypeId PartyTypeId,
    const FPartyInvitationRecipient &Recipient,
    const FOnRequestToJoinPartyComplete &Delegate)
{
    auto ErrorCode = Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("The EOS SDK does not support the RequestToJoinParty operation."));
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
    Delegate.ExecuteIfBound(
        LocalUserId,
        *Recipient.Id,
        FDateTime::MinValue(),
        ERequestToJoinPartyCompletionResult::UnknownInternalFailure);
}

void FOnlinePartySystemRedpointEOS::ClearRequestToJoinParty(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &Sender,
    EPartyRequestToJoinRemovedReason Reason)
{
    auto ErrorCode = Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("The EOS SDK does not support the ClearRequestToJoinParty operation."));
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
}

void FOnlinePartySystemRedpointEOS::QueryPartyJoinability(
    const FUniqueNetId &LocalUserId,
    const IOnlinePartyJoinInfo &OnlinePartyJoinInfo,
    const FOnQueryPartyJoinabilityCompleteEx &Delegate)
{
    auto ErrorCode = Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("The EOS SDK does not support the QueryPartyJoinability operation."));
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());

    FQueryPartyJoinabilityResult Result;
    Result.Result = AsOSSError(ErrorCode);
    Delegate.ExecuteIfBound(LocalUserId, OnlinePartyJoinInfo.GetPartyId().Get(), Result);
}

bool FOnlinePartySystemRedpointEOS::RejoinParty(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FOnlinePartyTypeId &PartyTypeId,
    const TArray<FUniqueNetIdRef> &FormerMembers,
    const FOnJoinPartyComplete &Delegate)
{
    auto ErrorCode = Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("The EOS SDK does not support the RejoinParty operation."));
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
    Delegate.ExecuteIfBound(LocalUserId, PartyId, EJoinPartyCompletionResult::UnknownInternalFailure, 0);
    return true;
}

bool FOnlinePartySystemRedpointEOS::LeaveParty(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FOnLeavePartyComplete &Delegate)
{
    return this->LeaveParty(LocalUserId, PartyId, false, Delegate);
}

bool FOnlinePartySystemRedpointEOS::LeaveParty(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    bool bSynchronizeLeave,
    const FOnLeavePartyComplete &Delegate)
{
    // Try to find the target room.
    auto CurrentRoom = this->GetUserCurrentRoom(LocalUserId, PartyId);
    if (!CurrentRoom.IsValid())
    {
        Delegate.ExecuteIfBound(LocalUserId, PartyId, ELeavePartyCompletionResult::UnknownParty);
        return true;
    }

    // Initialize and execute the leave room request.
    auto LeaveRequest = Requests::FLeaveRoomRequest();
    this->RoomSystem->LeaveRoom(
        CurrentRoom.ToSharedRef(),
        LeaveRequest,
        IRoomSystem::FOnLeaveRoomComplete::CreateSPLambda(
            this,
            [this, Delegate, UserId = LocalUserId.AsShared(), CurrentRoom](FError ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                    Delegate.ExecuteIfBound(
                        *UserId,
                        *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                        this->ConvertErrorToLeavePartyCompletionResult(ErrorCode));
                }
                else
                {
                    this->RemoveRoomFromPartyDatabaseIfPresent(GetAccountId(UserId), CurrentRoom.ToSharedRef());
                    Delegate.ExecuteIfBound(
                        *UserId,
                        *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                        ELeavePartyCompletionResult::Succeeded);
                }
            }));
    return true;
}

bool FOnlinePartySystemRedpointEOS::ApproveJoinRequest(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &RecipientId,
    bool bIsApproved,
    int32 DeniedResultCode)
{
    TSharedPtr<const FOnlinePartyRequestToJoinInfoRedpointEOS> SelectedRequest = nullptr;
    auto &Requests = this->RequestToJoins.FindOrAdd(LocalUserId.AsShared());
    for (const auto &Request : Requests)
    {
        auto RequestEOS = StaticCastSharedRef<const FOnlinePartyRequestToJoinInfoRedpointEOS>(Request);
        if (*RequestEOS->GetPartyId() == PartyId)
        {
            SelectedRequest = RequestEOS;
            break;
        }
    }
    if (!SelectedRequest.IsValid())
    {
        auto ErrorCode = Errors::NotFound(ANSI_TO_TCHAR(__FUNCTION__), TEXT("No such request to join exists."));
        UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
        return false;
    }

    if (bIsApproved)
    {
        this->JoinPartyOnRequest(SelectedRequest->GetRoomEventContext(), SelectedRequest->GetRoomRequestedJoin());
    }
    else
    {
        this->RoomSystem->RejectRoomRequestedJoin(
            SelectedRequest->GetRoomRequestedJoin(),
            IRoomSystem::FOnRejectRoomRequestedJoinComplete::CreateLambda([](const FError &ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                }
            }));
    }

    Requests.Remove(SelectedRequest.ToSharedRef());
    this->TriggerOnPartyRequestToJoinRemovedDelegates(
        *SelectedRequest->GetUserId(),
        *SelectedRequest->GetPartyId(),
        *SelectedRequest->GetUserId(),
        *SelectedRequest,
        bIsApproved ? EPartyRequestToJoinRemovedReason::Accepted : EPartyRequestToJoinRemovedReason::Dismissed);
    return true;
}

void FOnlinePartySystemRedpointEOS::RespondToQueryJoinability(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &RecipientId,
    bool bCanJoin,
    int32 DeniedResultCode,
    FOnlinePartyDataConstPtr PartyData)
{
    auto ErrorCode = Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("The EOS SDK does not support the RespondToQueryJoinability operation."));
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
}

bool FOnlinePartySystemRedpointEOS::SendInvitation(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FPartyInvitationRecipient &Recipient,
    const FOnSendPartyInvitationComplete &Delegate)
{
    // Ensure the local user is an EOS user.
    if (!LocalUserId.GetType().IsEqual(REDPOINT_EOS_SUBSYSTEM))
    {
        // The target user doesn't have a PUID, so we can't send them an invite over EOS.
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("Invalid local user ID passed to %s."),
            ANSI_TO_TCHAR(__FUNCTION__));
        Delegate.ExecuteIfBound(LocalUserId, PartyId, *Recipient.Id, ESendPartyInvitationCompletionResult::NotLoggedIn);
        return true;
    }

    // Try to find the target room to send the invite for.
    auto CurrentRoom = this->GetUserCurrentRoom(LocalUserId, PartyId);
    if (!CurrentRoom.IsValid())
    {
        Delegate.ExecuteIfBound(
            LocalUserId,
            PartyId,
            *Recipient.Id,
            ESendPartyInvitationCompletionResult::UnknownInternalFailure);
        return true;
    }

    // Our callback that will send the room invite request over EOS, *only* if we weren't able to invite the player
    // using the local platform.
    auto SendInvite = TDelegate<void(const FError &ErrorCode)>::CreateSPLambda(
        this,
        [this, CurrentRoom, UserId = LocalUserId.AsShared(), RecipientId = Recipient.Id, Delegate](
            const FError &ErrorCode) {
            if (ErrorCode.WasSuccessful())
            {
                // We successfully sent the invite over the local platform; we don't need to send it over EOS as well.
                Delegate.ExecuteIfBound(
                    *UserId,
                    *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                    *UserId,
                    ESendPartyInvitationCompletionResult::Succeeded);
                return;
            }

            if (!RecipientId->GetType().IsEqual(REDPOINT_EOS_SUBSYSTEM))
            {
                // The target user doesn't have a PUID, so we can't send them an invite over EOS.
                UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                Delegate.ExecuteIfBound(
                    *UserId,
                    *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                    *RecipientId,
                    this->ConvertErrorToSendPartyInvitationCompletionResult(ErrorCode));
                return;
            }

            // Initialize and execute the send room invite request.
            auto SendInviteRequest = Requests::FSendRoomInviteRequest();
            SendInviteRequest.TargetUser = GetAccountId(RecipientId);
            this->RoomSystem->SendRoomInvite(
                CurrentRoom.ToSharedRef(),
                SendInviteRequest,
                IRoomSystem::FOnSendRoomInviteComplete::CreateSPLambda(
                    this,
                    [this, Delegate, UserId, RecipientId, CurrentRoom](FError SendErrorCode) {
                        if (!SendErrorCode.WasSuccessful())
                        {
                            UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *SendErrorCode.ToLogString());
                            Delegate.ExecuteIfBound(
                                *UserId,
                                *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                                *RecipientId,
                                this->ConvertErrorToSendPartyInvitationCompletionResult(SendErrorCode));
                        }
                        else
                        {
                            Delegate.ExecuteIfBound(
                                *UserId,
                                *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                                *RecipientId,
                                ESendPartyInvitationCompletionResult::Succeeded);
                        }
                    }));
        });

    // First, ask the social implementation to send the invite to a friend, if possible.
    auto SocialPinned = this->Social.Pin();
    if (SocialPinned.IsValid())
    {
        SocialPinned->SendRoomInviteToPotentialFriend(LocalUserId, Recipient.Id, CurrentRoom.ToSharedRef(), SendInvite);
    }
    else
    {
        SendInvite.ExecuteIfBound(Errors::NoConnection(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("There are no local platform implementations available to send the room invite.")));
    }
    return true;
}

void FOnlinePartySystemRedpointEOS::CancelInvitation(
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &TargetUserId,
    const FOnlinePartyId &PartyId,
    const FOnCancelPartyInvitationComplete &Delegate)
{
    auto ErrorCode = Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("The EOS SDK does not support the CancelInvitation operation."));
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
    Delegate.ExecuteIfBound(LocalUserId, PartyId, TargetUserId, AsOSSError(ErrorCode));
}

bool FOnlinePartySystemRedpointEOS::RejectInvitation(const FUniqueNetId &LocalUserId, const FUniqueNetId &SenderId)
{
    // Try to find the invite to reject it.
    auto CurrentRoomInvites = this->RoomSystem->GetCurrentRoomInvites(GetAccountId(LocalUserId), NAME_Game);
    auto CurrentRoomInvitePtr =
        CurrentRoomInvites.FindByPredicate([SenderAccountId = GetAccountId(SenderId)](const FRoomInviteRef &Value) {
            return Value->GetRoomInviteSenderUserId() == SenderAccountId;
        });
    auto CurrentRoomInvite = CurrentRoomInvitePtr == nullptr ? FRoomInvitePtr() : *CurrentRoomInvitePtr;
    if (!CurrentRoomInvite.IsValid())
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("Unable to reject the invitation because it could not be found."));
        return false;
    }
    if (!CurrentRoomInvite->GetRoomInviteSearchResult()
             ->GetRoomFeatureSearchState<Features::IPartyTypeRoomFeatureSearchState>()
             .IsValid())
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("Unable to reject the invitation because it was not for a party."));
        return false;
    }

    // Reject the room invite.
    auto JoinInfo = MakeShared<FOnlinePartyJoinInfoRedpointEOSRoomInvite>(this->PlatformHandle, CurrentRoomInvite);
    this->RoomSystem->RejectRoomInvite(
        CurrentRoomInvite.ToSharedRef(),
        IRoomSystem::FOnRejectRoomInviteComplete::CreateSPLambda(
            this,
            [this, UserId = LocalUserId.AsShared(), JoinInfo](FError ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                    // @note: We can not propagate the error to the user because there is no event to encapsulate the
                    // failure of RejectInvitation in the IOnlinePartySystem API design.
                }
                else
                {
                    this->TriggerOnPartyInviteRemovedExDelegates(
                        *UserId,
                        *JoinInfo,
                        EPartyInvitationRemovedReason::Declined);
                }
            }));
    return true;
}

void FOnlinePartySystemRedpointEOS::ClearInvitations(
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &SenderId,
    const FOnlinePartyId *PartyId)
{
    auto ErrorCode = Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("The EOS SDK does not support the ClearInvitations operation."));
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
}

bool FOnlinePartySystemRedpointEOS::KickMember(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &TargetMemberId,
    const FOnKickPartyMemberComplete &Delegate)
{
    // Try to find the target room, and make sure the local user is the owner.
    auto CurrentRoom = this->GetUserCurrentRoom(LocalUserId, PartyId);
    if (!CurrentRoom.IsValid())
    {
        Delegate.ExecuteIfBound(LocalUserId, PartyId, TargetMemberId, EKickMemberCompletionResult::UnknownParty);
        return true;
    }
    if (CurrentRoom->GetRoomOwner()->IsDedicatedServer() ||
        CurrentRoom->GetRoomOwner()->GetUserId() != GetAccountId(LocalUserId))
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("You can not kick members from this party because you are not the leader of it."));
        Delegate
            .ExecuteIfBound(LocalUserId, PartyId, TargetMemberId, EKickMemberCompletionResult::LocalMemberNotLeader);
        return true;
    }

    // Initialize and execute the kick room member request.
    auto KickMemberRequest = Requests::FKickRoomMemberRequest();
    KickMemberRequest.TargetUser = GetAccountId(TargetMemberId);
    this->RoomSystem->KickRoomMember(
        CurrentRoom.ToSharedRef(),
        KickMemberRequest,
        IRoomSystem::FOnKickRoomMemberComplete::CreateSPLambda(
            this,
            [this, Delegate, UserId = LocalUserId.AsShared(), MemberId = TargetMemberId.AsShared(), CurrentRoom](
                FError ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                    Delegate.ExecuteIfBound(
                        *UserId,
                        *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                        *MemberId,
                        this->ConvertErrorToKickMemberCompletionResult(ErrorCode));
                }
                else
                {
                    Delegate.ExecuteIfBound(
                        *UserId,
                        *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                        *MemberId,
                        EKickMemberCompletionResult::Succeeded);
                }
            }));
    return true;
}

bool FOnlinePartySystemRedpointEOS::PromoteMember(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &TargetMemberId,
    const FOnPromotePartyMemberComplete &Delegate)
{
    // Try to find the target room, and make sure the local user is the owner.
    auto CurrentRoom = this->GetUserCurrentRoom(LocalUserId, PartyId);
    if (!CurrentRoom.IsValid())
    {
        Delegate.ExecuteIfBound(LocalUserId, PartyId, TargetMemberId, EPromoteMemberCompletionResult::UnknownParty);
        return true;
    }
    if (CurrentRoom->GetRoomOwner()->IsDedicatedServer() ||
        CurrentRoom->GetRoomOwner()->GetUserId() != GetAccountId(LocalUserId))
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("You can not promote someone else to be the leader from this party because you are not the leader of "
                 "it."));
        Delegate
            .ExecuteIfBound(LocalUserId, PartyId, TargetMemberId, EPromoteMemberCompletionResult::LocalMemberNotLeader);
        return true;
    }

    // Initialize and execute the promote room member request.
    auto PromoteMemberRequest = Requests::FPromoteRoomMemberRequest();
    PromoteMemberRequest.TargetUser = GetAccountId(TargetMemberId);
    this->RoomSystem->PromoteRoomMember(
        CurrentRoom.ToSharedRef(),
        PromoteMemberRequest,
        IRoomSystem::FOnPromoteRoomMemberComplete::CreateSPLambda(
            this,
            [this, Delegate, UserId = LocalUserId.AsShared(), MemberId = TargetMemberId.AsShared(), CurrentRoom](
                FError ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
                    Delegate.ExecuteIfBound(
                        *UserId,
                        *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                        *MemberId,
                        this->ConvertErrorToPromoteMemberCompletionResult(ErrorCode));
                }
                else
                {
                    Delegate.ExecuteIfBound(
                        *UserId,
                        *MakeShared<FOnlinePartyIdRedpointEOS>(CurrentRoom->GetRoomId()),
                        *MemberId,
                        EPromoteMemberCompletionResult::Succeeded);
                }
            }));
    return true;
}

bool FOnlinePartySystemRedpointEOS::UpdatePartyData(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &Namespace,
    const FOnlinePartyData &UnsafePartyData)
{
    TSharedRef<const FOnlinePartyData> HeapPartyData = UnsafePartyData.DoesSharedInstanceExist()
                                                           ? UnsafePartyData.AsShared()
                                                           : MakeShared<FOnlinePartyData>(UnsafePartyData);
    const FOnlinePartyData &PartyData = *HeapPartyData;

    // Try to find the target room, and make sure the local user is the owner.
    auto CurrentRoom = this->GetUserCurrentRoom(LocalUserId, PartyId);
    if (!CurrentRoom.IsValid())
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s"),
            *Errors::NotFound(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 TEXT("Unable to locate party to update it. Did you leave it?"))
                 .ToLogString());
        return false;
    }
    if (CurrentRoom->GetRoomOwner()->IsDedicatedServer() ||
        CurrentRoom->GetRoomOwner()->GetUserId() != GetAccountId(LocalUserId))
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s"),
            *Errors::InvalidRequest(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 TEXT("You can not update this party because you are not the leader of it."))
                 .ToLogString());
        return false;
    }

    // Initialize the room update request.
    auto UpdateRequest = Requests::FUpdateRoomRequest();

    // Apply the requested changes to the update request.
    FOnlineKeyValuePairs<FString, FVariantData> UpsertKV;
    TArray<FString> DeleteK;
    PartyData.GetDirtyKeyValAttrs(UpsertKV, DeleteK);
    for (const auto &KV : UpsertKV)
    {
        FRoomAttribute Attribute = FRoomAttribute();
        switch (KV.Value.GetType())
        {
        case EOnlineKeyValuePairDataType::String: {
            FString OutValue;
            KV.Value.GetValue(OutValue);
            Attribute = FRoomAttribute(OutValue, FRoomAttribute::EVisibility::Public);
            break;
        }
        case EOnlineKeyValuePairDataType::Int64: {
            int64 OutValue;
            KV.Value.GetValue(OutValue);
            Attribute = FRoomAttribute(OutValue, FRoomAttribute::EVisibility::Public);
            break;
        }
        case EOnlineKeyValuePairDataType::Double: {
            double OutValue;
            KV.Value.GetValue(OutValue);
            Attribute = FRoomAttribute(OutValue, FRoomAttribute::EVisibility::Public);
            break;
        }
        case EOnlineKeyValuePairDataType::Bool: {
            bool OutValue;
            KV.Value.GetValue(OutValue);
            Attribute = FRoomAttribute(OutValue, FRoomAttribute::EVisibility::Public);
            break;
        }
        default: {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Error,
                TEXT("%s"),
                *Errors::InvalidRequest(
                     ANSI_TO_TCHAR(__FUNCTION__),
                     FString::Printf(
                         TEXT("Attribute '%s' has an unsupported type. EOS only supports String, Int64, Double and "
                              "Bool."),
                         *KV.Key))
                     .ToLogString());
            return false;
        }
        }
        UpdateRequest.UpsertAttributes.Add(KV.Key, Attribute);
    }
    for (const auto &Key : DeleteK)
    {
        UpdateRequest.DeleteAttributes.Add(Key);
    }

    // Since OSSv1 does not have a normal callback from UpdatePartyData, we need to ensure that the global events
    // *always* fire, even when the UpdatePartyData call isn't making any effective changes (you could have multiple
    // UpdatePartyData calls racing, so we can't just check for actual changes here and return false). To do this, we
    // set a hidden attribute just for OSSv1 to a random value that ensures the event always fires.
    UpdateRequest.UpsertAttributes.Add(TEXT("RedpointOSSv1EventForcingAttribute"), FGuid::NewGuid().ToString());

    // Update the room for the party.
    this->RoomSystem->UpdateRoom(
        CurrentRoom.ToSharedRef(),
        UpdateRequest,
        IRoomSystem::FOnUpdateRoomComplete::CreateSPLambda(this, [](FError ErrorCode) {
            if (!ErrorCode.WasSuccessful())
            {
                UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
            }

            // @note: The online subsystem APIs have no way of passing operation completion on for UpdatePartyData.
        }));
    return true;
}

bool FOnlinePartySystemRedpointEOS::UpdatePartyMemberData(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &Namespace,
    const FOnlinePartyData &UnsafePartyMemberData)
{
    TSharedRef<const FOnlinePartyData> HeapPartyMemberData = UnsafePartyMemberData.DoesSharedInstanceExist()
                                                                 ? UnsafePartyMemberData.AsShared()
                                                                 : MakeShared<FOnlinePartyData>(UnsafePartyMemberData);
    const FOnlinePartyData &PartyMemberData = *HeapPartyMemberData;

    // Try to find the target room.
    auto CurrentRoom = this->GetUserCurrentRoom(LocalUserId, PartyId);
    if (!CurrentRoom.IsValid())
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s"),
            *Errors::NotFound(
                 ANSI_TO_TCHAR(__FUNCTION__),
                 TEXT("Unable to locate party to update it. Did you leave it?"))
                 .ToLogString());
        return false;
    }

    // Initialize the room update request.
    auto UpdateRequest = Requests::FUpdateRoomRequest();

    // Apply the requested changes to the update request.
    FOnlineKeyValuePairs<FString, FVariantData> UpsertKV;
    TArray<FString> DeleteK;
    PartyMemberData.GetDirtyKeyValAttrs(UpsertKV, DeleteK);
    for (const auto &KV : UpsertKV)
    {
        FRoomAttribute Attribute = FRoomAttribute();
        switch (KV.Value.GetType())
        {
        case EOnlineKeyValuePairDataType::String: {
            FString OutValue;
            KV.Value.GetValue(OutValue);
            Attribute = FRoomAttribute(OutValue, FRoomAttribute::EVisibility::Public);
            break;
        }
        case EOnlineKeyValuePairDataType::Int64: {
            int64 OutValue;
            KV.Value.GetValue(OutValue);
            Attribute = FRoomAttribute(OutValue, FRoomAttribute::EVisibility::Public);
            break;
        }
        case EOnlineKeyValuePairDataType::Double: {
            double OutValue;
            KV.Value.GetValue(OutValue);
            Attribute = FRoomAttribute(OutValue, FRoomAttribute::EVisibility::Public);
            break;
        }
        case EOnlineKeyValuePairDataType::Bool: {
            bool OutValue;
            KV.Value.GetValue(OutValue);
            Attribute = FRoomAttribute(OutValue, FRoomAttribute::EVisibility::Public);
            break;
        }
        default: {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Error,
                TEXT("%s"),
                *Errors::InvalidRequest(
                     ANSI_TO_TCHAR(__FUNCTION__),
                     FString::Printf(
                         TEXT("Attribute '%s' has an unsupported type. EOS only supports String, Int64, Double and "
                              "Bool."),
                         *KV.Key))
                     .ToLogString());
            return false;
        }
        }
        UpdateRequest.UpsertMemberAttributes.Add(KV.Key, Attribute);
    }
    for (const auto &Key : DeleteK)
    {
        UpdateRequest.DeleteMemberAttributes.Add(Key);
    }

    // Since OSSv1 does not have a normal callback from UpdatePartyMemberData, we need to ensure that the global events
    // *always* fire, even when the UpdatePartyMemberData call isn't making any effective changes (you could have
    // multiple UpdatePartyMemberData calls racing, so we can't just check for actual changes here and return false). To
    // do this, we set a hidden attribute just for OSSv1 to a random value that ensures the event always fires.
    UpdateRequest.UpsertMemberAttributes.Add(TEXT("RedpointOSSv1EventForcingAttribute"), FGuid::NewGuid().ToString());

    // Update the room for the party.
    this->RoomSystem->UpdateRoom(
        CurrentRoom.ToSharedRef(),
        UpdateRequest,
        IRoomSystem::FOnUpdateRoomComplete::CreateSPLambda(this, [](FError ErrorCode) {
            if (!ErrorCode.WasSuccessful())
            {
                UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
            }

            // @note: The online subsystem APIs have no way of passing operation completion on for
            // UpdatePartyMemberData.
        }));
    return true;
}

bool FOnlinePartySystemRedpointEOS::IsMemberLeader(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId) const
{
    auto CurrentRoom = this->GetUserCurrentRoom(LocalUserId, PartyId);
    if (!CurrentRoom.IsValid())
    {
        return false;
    }
    return !CurrentRoom->GetRoomOwner()->IsDedicatedServer() &&
           CurrentRoom->GetRoomOwner()->GetUserId() == GetAccountId(MemberId);
}

uint32 FOnlinePartySystemRedpointEOS::GetPartyMemberCount(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId) const
{
    auto CurrentRoom = this->GetUserCurrentRoom(LocalUserId, PartyId);
    if (!CurrentRoom.IsValid())
    {
        return false;
    }
    return static_cast<uint32>(CurrentRoom->GetMembers().Num());
}

FOnlinePartyConstPtr FOnlinePartySystemRedpointEOS::GetParty(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId) const
{
    FIndexedRowConstPtr IndexedRow =
        this->PartyDatabase.FindFirstByValue(CPartyDatabaseLocalUserIdAndPartyId.Create(FUniqueNetIdAndPartyId(
            LocalUserId.AsShared(),
            StaticCastSharedRef<const FOnlinePartyIdRedpointEOS>(PartyId.AsShared()))));
    if (!IndexedRow.IsValid())
    {
        return nullptr;
    }
    return CPartyDatabaseOnlinePartyRedpointEOSRef.Get(IndexedRow.ToSharedRef());
}

FOnlinePartyConstPtr FOnlinePartySystemRedpointEOS::GetParty(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyTypeId &PartyTypeId) const
{
    FIndexedRowConstPtr IndexedRow =
        this->PartyDatabase.FindFirstByValue(CPartyDatabaseLocalUserIdAndPartyTypeId.Create(
            FUniqueNetIdAndPartyTypeId(LocalUserId.AsShared(), PartyTypeId)));
    if (!IndexedRow.IsValid())
    {
        return nullptr;
    }
    return CPartyDatabaseOnlinePartyRedpointEOSRef.Get(IndexedRow.ToSharedRef());
}

FOnlinePartyMemberConstPtr FOnlinePartySystemRedpointEOS::GetPartyMember(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId) const
{
    FIndexedRowConstPtr IndexedRow = this->PartyMemberDatabase.FindFirstByValue(
        CPartyMemberDatabaseLocalUserIdAndPartyIdAndMemberId.Create(FUniqueNetIdAndPartyIdAndMemberId(
            LocalUserId.AsShared(),
            StaticCastSharedRef<const FOnlinePartyIdRedpointEOS>(PartyId.AsShared()),
            MemberId.AsShared())));
    if (!IndexedRow.IsValid())
    {
        return nullptr;
    }
    return CPartyMemberDatabaseOnlinePartyMemberRedpointEOSRef.Get(IndexedRow.ToSharedRef());
}

FOnlinePartyDataConstPtr FOnlinePartySystemRedpointEOS::GetPartyData(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &Namespace) const
{
    FIndexedRowConstPtr IndexedRow =
        this->PartyDatabase.FindFirstByValue(CPartyDatabaseLocalUserIdAndPartyId.Create(FUniqueNetIdAndPartyId(
            LocalUserId.AsShared(),
            StaticCastSharedRef<const FOnlinePartyIdRedpointEOS>(PartyId.AsShared()))));
    if (!IndexedRow.IsValid())
    {
        return nullptr;
    }
    TSharedRef<FOnlinePartyRedpointEOS> PartyRedpointEOS =
        CPartyDatabaseOnlinePartyRedpointEOSRef.Get(IndexedRow.ToSharedRef());
    return PartyRedpointEOS->GetPartyData(Namespace);
}

FOnlinePartyDataConstPtr FOnlinePartySystemRedpointEOS::GetPartyMemberData(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    const FName &Namespace) const
{
    FIndexedRowConstPtr IndexedRow = this->PartyMemberDatabase.FindFirstByValue(
        CPartyMemberDatabaseLocalUserIdAndPartyIdAndMemberId.Create(FUniqueNetIdAndPartyIdAndMemberId(
            LocalUserId.AsShared(),
            StaticCastSharedRef<const FOnlinePartyIdRedpointEOS>(PartyId.AsShared()),
            MemberId.AsShared())));
    if (!IndexedRow.IsValid())
    {
        return nullptr;
    }
    TSharedRef<FOnlinePartyMemberRedpointEOS> PartyMemberRedpointEOS =
        CPartyMemberDatabaseOnlinePartyMemberRedpointEOSRef.Get(IndexedRow.ToSharedRef());
    return PartyMemberRedpointEOS->GetPartyMemberData(Namespace);
}

IOnlinePartyJoinInfoConstPtr FOnlinePartySystemRedpointEOS::GetAdvertisedParty(
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &UserId,
    const FOnlinePartyTypeId PartyTypeId) const
{
    if (PartyTypeId != this->GetPrimaryPartyTypeId())
    {
        // Only parties that have a primary party type ID are advertised via presence, therefore if the requested party
        // type ID isn't the primary party type ID, then there can't be any advertised party.
        return nullptr;
    }

    auto PresenceSystem = this->PlatformHandle->GetSystem<IPresenceSystem>();

    auto CurrentPresence = PresenceSystem->GetPresence(GetAccountId(LocalUserId), GetAccountId(UserId));
    if (!CurrentPresence.IsSet() || !CurrentPresence->AdvertisedParty.IsSet() ||
        !CurrentPresence->AdvertisedParty->bJoinable)
    {
        // We don't have presence info for the target user, they're not advertising a party, or the party they're
        // advertising isn't joinable.
        return nullptr;
    }

    return MakeShared<FOnlinePartyJoinInfoRedpointEOSRoomId>(CurrentPresence->AdvertisedParty->RoomId, true);
}

bool FOnlinePartySystemRedpointEOS::GetJoinedParties(
    const FUniqueNetId &LocalUserId,
    TArray<TSharedRef<const FOnlinePartyId>> &OutPartyIdArray) const
{
    OutPartyIdArray.Empty();
    auto PartyRows = this->PartyDatabase.FindByValue(CPartyDatabaseLocalUserId.Create(LocalUserId.AsShared()));
    for (const auto &PartyRow : PartyRows)
    {
        OutPartyIdArray.Add(CPartyDatabaseOnlinePartyRedpointEOSRef.Get(PartyRow)->PartyId);
    }
    return true;
}

bool FOnlinePartySystemRedpointEOS::GetPartyMembers(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    TArray<FOnlinePartyMemberConstRef> &OutPartyMembersArray) const
{
    OutPartyMembersArray.Empty();
    auto PartyMemberRows =
        this->PartyMemberDatabase.FindByValue(CPartyMemberDatabaseLocalUserIdAndPartyId.Create(FUniqueNetIdAndPartyId(
            LocalUserId.AsShared(),
            StaticCastSharedRef<const FOnlinePartyIdRedpointEOS>(PartyId.AsShared()))));
    for (const auto &PartyMemberRow : PartyMemberRows)
    {
        OutPartyMembersArray.Add(CPartyMemberDatabaseOnlinePartyMemberRedpointEOSRef.Get(PartyMemberRow));
    }
    return true;
}

bool FOnlinePartySystemRedpointEOS::GetPendingInvites(
    const FUniqueNetId &LocalUserId,
    TArray<IOnlinePartyJoinInfoConstRef> &OutPendingInvitesArray) const
{
    OutPendingInvitesArray.Empty();
    auto CurrentRoomInvites = this->RoomSystem->GetCurrentRoomInvites(GetAccountId(LocalUserId), NAME_Game);
    for (const auto &CurrentRoomInvite : CurrentRoomInvites)
    {
        OutPendingInvitesArray.Add(
            MakeShared<FOnlinePartyJoinInfoRedpointEOSRoomInvite>(this->PlatformHandle, CurrentRoomInvite));
    }
    return true;
}

bool FOnlinePartySystemRedpointEOS::GetPendingJoinRequests(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    TArray<IOnlinePartyPendingJoinRequestInfoConstRef> &OutPendingJoinRequestArray) const
{
    auto ErrorCode = Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("The EOS SDK does not support the GetPendingJoinRequests operation."));
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
    return false;
}

bool FOnlinePartySystemRedpointEOS::GetPendingInvitedUsers(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    TArray<FUniqueNetIdRef> &OutPendingInvitedUserArray) const
{
    auto ErrorCode = Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("The EOS SDK does not support the GetPendingInvitedUsers operation."));
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
    return false;
}

bool FOnlinePartySystemRedpointEOS::GetPendingRequestsToJoin(
    const FUniqueNetId &LocalUserId,
    TArray<IOnlinePartyRequestToJoinInfoConstRef> &OutRequestsToJoin) const
{
    auto *Requests = this->RequestToJoins.Find(LocalUserId.AsShared());
    if (Requests != nullptr)
    {
        OutRequestsToJoin = *Requests;
    }
    else
    {
        OutRequestsToJoin.Empty();
    }
    return true;
}

FString FOnlinePartySystemRedpointEOS::MakeJoinInfoJson(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
{
    auto LocalUserIdEOS = StaticCastSharedRef<const FUniqueNetIdEOS>(LocalUserId.AsShared());
    auto PartyIdEOS = StaticCastSharedRef<const FOnlinePartyIdRedpointEOS>(PartyId.AsShared());

    TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
    Json->SetStringField(TEXT("LocalUserId"), LocalUserIdEOS->GetProductUserIdString());
    Json->SetStringField(TEXT("PartyId"), PartyIdEOS->GetRoomId()->GetRoomId());
    Json->SetBoolField(TEXT("IsPrimaryParty"), false /* @todo */);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Json, Writer);
    return OutputString;
}

IOnlinePartyJoinInfoConstPtr FOnlinePartySystemRedpointEOS::MakeJoinInfoFromJson(const FString &JoinInfoJson)
{
    TSharedPtr<FJsonObject> Json;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JoinInfoJson);
    FJsonSerializer::Deserialize(*Reader, Json);

    if (!Json->HasField(TEXT("LocalUserId")) || !Json->HasField(TEXT("PartyId")))
    {
        return nullptr;
    }

    FString LocalUserIdStr = Json->GetStringField(TEXT("LocalUserId"));
    FString PartyIdStr = Json->GetStringField(TEXT("PartyId"));
    bool IsPrimaryParty = false;
    Json->TryGetBoolField(TEXT("IsPrimaryParty"), IsPrimaryParty);

    return MakeShared<FOnlinePartyJoinInfoRedpointEOSRoomId>(
        MakeShared<FRoomId>(FName(TEXT("Lobby")), PartyIdStr),
        IsPrimaryParty);
}

FString FOnlinePartySystemRedpointEOS::MakeTokenFromJoinInfo(const IOnlinePartyJoinInfo &JoinInfo) const
{
    auto ErrorCode = Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("The EOS SDK does not support the MakeTokenFromJoinInfo operation."));
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
    return TEXT("");
}

IOnlinePartyJoinInfoConstPtr FOnlinePartySystemRedpointEOS::MakeJoinInfo(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId)
{
    auto PartyIdRedpointEOS = StaticCastSharedRef<const FOnlinePartyIdRedpointEOS>(PartyId.AsShared());
    return MakeShared<FOnlinePartyJoinInfoRedpointEOSRoomId>(PartyIdRedpointEOS->GetRoomId().ToSharedRef(), false);
}

IOnlinePartyJoinInfoConstPtr FOnlinePartySystemRedpointEOS::MakeJoinInfoFromToken(const FString &Token) const
{
    auto ErrorCode = Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("The EOS SDK does not support the MakeJoinInfoFromToken operation."));
    UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("%s"), *ErrorCode.ToLogString());
    return nullptr;
}

IOnlinePartyJoinInfoConstPtr FOnlinePartySystemRedpointEOS::ConsumePendingCommandLineInvite()
{
    // @todo
    return nullptr;
}

void FOnlinePartySystemRedpointEOS::DumpPartyState()
{
    {
        const auto &Rows = this->PartyDatabase.GetAllRows();
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("Parties: (%d %s)"),
            Rows.Num(),
            Rows.Num() == 1 ? TEXT("party") : TEXT("parties"));
        int i = 0;
        for (const auto &Row : Rows)
        {
            i++;
            auto LocalUserId = CPartyDatabaseLocalUserId.Get(Row);
            auto LocalUserIdAndPartyId = CPartyDatabaseLocalUserIdAndPartyId.Get(Row);
            auto LocalUserIdAndPartyTypeId = CPartyDatabaseLocalUserIdAndPartyTypeId.Get(Row);
            auto RoomRef = CPartyDatabaseRoomRef.Get(Row);
            auto OnlinePartyRedpointEOSRef = CPartyDatabaseOnlinePartyRedpointEOSRef.Get(Row);
            auto PartyMemberJoinedHandle = CPartyDatabasePartyMemberJoinedHandle.Get(Row);
            auto PartyMemberLeftHandle = CPartyDatabasePartyMemberLeftHandle.Get(Row);
            UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("Party #%d:"), i);
            UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("  Local User ID: %s"), *LocalUserId->ToString());
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("  Local User ID and Party ID: (%s, %s)"),
                *LocalUserIdAndPartyId.Key->ToString(),
                *LocalUserIdAndPartyId.Value->ToString());
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("  Local User ID and Party Type ID: (%s, %u)"),
                *LocalUserIdAndPartyTypeId.Key->ToString(),
                LocalUserIdAndPartyTypeId.Value.GetValue());
            UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("  Room:"));
            UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("    Room ID: %s"), *RoomRef->GetRoomId()->ToString());
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    Bound For Local User ID: %s"),
                *GetUserIdString(RoomRef->GetBoundForLocalUser()));
            if (RoomRef->GetRoomOwner()->IsDedicatedServer())
            {
                UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("    Owner User ID: (dedicated server)"));
            }
            else
            {
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    Verbose,
                    TEXT("    Owner User ID: %s"),
                    *GetUserIdString(RoomRef->GetRoomOwner()->GetUserId()));
            }
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    Maximum Members: %u"),
                RoomRef->GetRoomMaxMembers());
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    Features: (%d %s)"),
                RoomRef->GetRoomFeatureNames().Num(),
                RoomRef->GetRoomFeatureNames().Num() == 1 ? TEXT("feature") : TEXT("features"));
            for (const auto &Name : RoomRef->GetRoomFeatureNames())
            {
                UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("      %s"), *Name.ToString());
            }
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    Room Attributes: (%d %s)"),
                RoomRef->GetRoomAttributes().Num(),
                RoomRef->GetRoomAttributes().Num() == 1 ? TEXT("attribute") : TEXT("attributes"));
            for (const auto &KV : RoomRef->GetRoomAttributes())
            {
                switch (KV.Value.Type())
                {
                case FRoomAttribute::EType::Bool:
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Verbose,
                        TEXT("      %s = (bool) %s"),
                        *KV.Key,
                        KV.Value.GetBool() ? TEXT("TRUE") : TEXT("FALSE"));
                    break;
                case FRoomAttribute::EType::Int64:
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Verbose,
                        TEXT("      %s = (int64) %lld"),
                        *KV.Key,
                        KV.Value.GetInt64());
                    break;
                case FRoomAttribute::EType::Double:
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Verbose,
                        TEXT("      %s = (double) %f"),
                        *KV.Key,
                        KV.Value.GetDouble());
                    break;
                case FRoomAttribute::EType::String:
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Verbose,
                        TEXT("      %s = (string) %s"),
                        *KV.Key,
                        *KV.Value.GetString());
                    break;
                default:
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("      %s = (unknown)"), *KV.Key);
                    break;
                }
            }
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    Members: (%d %s)"),
                RoomRef->GetMembers().Num(),
                RoomRef->GetMembers().Num() == 1 ? TEXT("member") : TEXT("members"));
            for (const auto &Member : RoomRef->GetMembers())
            {
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    Verbose,
                    TEXT("      %s: (%d %s)"),
                    *GetUserIdString(Member->GetUserId()),
                    Member->GetAttributes().Num(),
                    Member->GetAttributes().Num() == 1 ? TEXT("member attribute") : TEXT("member attributes"));
                for (const auto &KV : Member->GetAttributes())
                {
                    switch (KV.Value.Type())
                    {
                    case FRoomAttribute::EType::Bool:
                        UE_LOG(
                            LogOnlineSubsystemRedpointEOS,
                            Verbose,
                            TEXT("        %s = (bool) %s"),
                            *KV.Key,
                            KV.Value.GetBool() ? TEXT("TRUE") : TEXT("FALSE"));
                        break;
                    case FRoomAttribute::EType::Int64:
                        UE_LOG(
                            LogOnlineSubsystemRedpointEOS,
                            Verbose,
                            TEXT("        %s = (int64) %lld"),
                            *KV.Key,
                            KV.Value.GetInt64());
                        break;
                    case FRoomAttribute::EType::Double:
                        UE_LOG(
                            LogOnlineSubsystemRedpointEOS,
                            Verbose,
                            TEXT("        %s = (double) %f"),
                            *KV.Key,
                            KV.Value.GetDouble());
                        break;
                    case FRoomAttribute::EType::String:
                        UE_LOG(
                            LogOnlineSubsystemRedpointEOS,
                            Verbose,
                            TEXT("        %s = (string) %s"),
                            *KV.Key,
                            *KV.Value.GetString());
                        break;
                    default:
                        UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("        %s = (unknown)"), *KV.Key);
                        break;
                    }
                }
            }
            UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("  OnlinePartyRedpointEOS:"));
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    Party ID: %s"),
                *OnlinePartyRedpointEOSRef->PartyId->ToString());
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    Leader ID: %s"),
                *OnlinePartyRedpointEOSRef->LeaderId->ToString());
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    Party Type ID: %u"),
                OnlinePartyRedpointEOSRef->PartyTypeId.GetValue());
        }
    }
    {
        const auto &Rows = this->PartyMemberDatabase.GetAllRows();
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("Party Members: (%d %s)"),
            Rows.Num(),
            Rows.Num() == 1 ? TEXT("party member") : TEXT("party members"));
        int i = 0;
        for (const auto &Row : Rows)
        {
            i++;
            auto LocalUserIdAndPartyId = CPartyMemberDatabaseLocalUserIdAndPartyId.Get(Row);
            auto LocalUserIdAndPartyIdAndMemberId = CPartyMemberDatabaseLocalUserIdAndPartyIdAndMemberId.Get(Row);
            auto RoomMemberRef = CPartyMemberDatabaseRoomMemberRef.Get(Row);
            auto OnlinePartyMemberRedpointEOSRef = CPartyMemberDatabaseOnlinePartyMemberRedpointEOSRef.Get(Row);
            UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("Party Member #%d:"), i);
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("  Local User ID and Party ID: (%s, %s)"),
                *LocalUserIdAndPartyId.Key->ToString(),
                *LocalUserIdAndPartyId.Value->ToString());
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("  Local User ID and Party ID and Member User ID: (%s, %s, %s)"),
                *LocalUserIdAndPartyIdAndMemberId.Get<0>()->ToString(),
                *LocalUserIdAndPartyIdAndMemberId.Get<1>()->ToString(),
                *LocalUserIdAndPartyIdAndMemberId.Get<2>()->ToString());
            UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("  Room Member:"));
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    User ID: %s"),
                *GetUserIdString(RoomMemberRef->GetUserId()));
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    Attributes: (%d %s)"),
                RoomMemberRef->GetAttributes().Num(),
                RoomMemberRef->GetAttributes().Num() == 1 ? TEXT("member attribute") : TEXT("member attributes"));
            for (const auto &KV : RoomMemberRef->GetAttributes())
            {
                switch (KV.Value.Type())
                {
                case FRoomAttribute::EType::Bool:
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Verbose,
                        TEXT("        %s = (bool) %s"),
                        *KV.Key,
                        KV.Value.GetBool() ? TEXT("TRUE") : TEXT("FALSE"));
                    break;
                case FRoomAttribute::EType::Int64:
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Verbose,
                        TEXT("        %s = (int64) %lld"),
                        *KV.Key,
                        KV.Value.GetInt64());
                    break;
                case FRoomAttribute::EType::Double:
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Verbose,
                        TEXT("        %s = (double) %f"),
                        *KV.Key,
                        KV.Value.GetDouble());
                    break;
                case FRoomAttribute::EType::String:
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Verbose,
                        TEXT("        %s = (string) %s"),
                        *KV.Key,
                        *KV.Value.GetString());
                    break;
                default:
                    UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("        %s = (unknown)"), *KV.Key);
                    break;
                }
            }
            UE_LOG(LogOnlineSubsystemRedpointEOS, Verbose, TEXT("  OnlinePartyMemberRedpointEOS:"));
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    User ID: %s"),
                *OnlinePartyMemberRedpointEOSRef->GetUserId()->ToString());
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    Display Name: %s"),
                *OnlinePartyMemberRedpointEOSRef->GetDisplayName());
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("    Real Name: %s"),
                *OnlinePartyMemberRedpointEOSRef->GetDisplayName());
        }
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()