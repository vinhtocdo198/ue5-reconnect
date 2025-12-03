// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFramework/CreateAutomaticParty.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "LogRedpointEOSFramework.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSRooms/Features/BucketRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/JoinByIdRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeatureRequest.h"
#include "RedpointEOSRooms/RoomSystem.h"
#include "RedpointFrameworkSubsystem.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1074738087, Redpoint::EOS::Framework)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Rooms::Requests;

void GetAutomaticPartySettings(
    const FPlatformHandle &PlatformHandle,
    bool &bOutEnsureAlwaysInParty,
    int32 &OutPartyMaximumSize,
    bool &bOutEnableVoiceChatInParty)
{
    bOutEnsureAlwaysInParty = false;
    OutPartyMaximumSize = 4;
    bOutEnableVoiceChatInParty = false;
    {
        URedpointFrameworkSubsystem *FrameworkSubsystem = nullptr;
        TSoftObjectPtr<UWorld> World = FWorldResolution::GetWorld(PlatformHandle->GetEnvironment()->InstanceName, true);
        if (World.IsValid() && IsValid(World->GetGameInstance()))
        {
            FrameworkSubsystem = World->GetGameInstance()->GetSubsystem<URedpointFrameworkSubsystem>();
        }
        if (!IsValid(FrameworkSubsystem))
        {
            // This happens if we are doing play-in-editor with "login before PIE". There is no world or game instance
            // yet, so directly grab the default settings from configuration.
            bOutEnsureAlwaysInParty =
                PlatformHandle->GetEnvironment()->Config->GetFrameworkDefaultEnsureAlwaysInParty();
            OutPartyMaximumSize = PlatformHandle->GetEnvironment()->Config->GetFrameworkDefaultPartyMaximumSize();
            bOutEnableVoiceChatInParty =
                PlatformHandle->GetEnvironment()->Config->GetFrameworkDefaultEnableVoiceChatInParty();
        }
        else
        {
            // Get the settings from the framework subsystem on the game instance.
            bOutEnsureAlwaysInParty = FrameworkSubsystem->bEnsureAlwaysInParty;
            OutPartyMaximumSize = FrameworkSubsystem->PartyMaximumSize;
            bOutEnableVoiceChatInParty = FrameworkSubsystem->bEnableVoiceChatInParty;
        }
    }

    OutPartyMaximumSize = FMath::Clamp(OutPartyMaximumSize, 1, 64);
}

void CreateAutomaticParty(
    int32 PartySize,
    bool bEnableVoiceChatInParty,
    const TSharedRef<IRoomSystem> &RoomSystem,
    const FRoomUserId &LocalUserId,
    const IRoomSystem::FOnCreateRoomComplete &OnComplete)
{
    auto Request = FCreateRoomRequest(
        ERoomScope::User,
        LocalUserId,
        FName(TEXT("Lobby")),
        NAME_Game,
        static_cast<uint32>(PartySize));
    Request.AddFeatureRequest(MakeShared<FBucketRoomFeatureRequest>(TEXT("Parties")));
    Request.AddFeatureRequest(MakeShared<FPartyConfigurationRoomFeatureRequest>(
        EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove,
        EPartyConfigurationRoomFeaturePermissionType::Anyone,
        EPartyConfigurationRoomFeaturePermissionType::Anyone,
        TEXT(""),
        TEXT(""),
        TEXT("")));
    Request.AddFeatureRequest(MakeShared<FPartyTypeRoomFeatureRequest>(PartyTypePrimaryPartyId));
    Request.AddFeatureRequest(MakeShared<FInvitesRoomFeatureRequest>(true));
    Request.AddFeatureRequest(MakeShared<FJoinByIdRoomFeatureRequest>());
    Request.AddFeatureRequest(MakeShared<FPublicMemberListRoomFeatureRequest>());
    Request.AddFeatureRequest(MakeShared<FOwnerMigrationRoomFeatureRequest>());
    Request.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
    Request.AddFeatureRequest(MakeShared<FPublicAdvertisementRoomFeatureRequest>());
    if (bEnableVoiceChatInParty)
    {
        Request.AddFeatureRequest(
            MakeShared<FVoiceChatRoomFeatureRequest>(RTC::EJoinRoomOptions::None, false, false, false));
    }
    RoomSystem->CreateRoom(
        Request,
        IRoomSystem::FOnCreateRoomComplete::CreateLambda([OnComplete](const FError &ErrorCode, const FRoomPtr &Room) {
            if (!ErrorCode.WasSuccessful())
            {
                UE_LOG(
                    LogRedpointEOSFramework,
                    Error,
                    TEXT("Unable to create automatic presence-enabled party in response to room leave: %s"),
                    *ErrorCode.ToLogString());
            }
            OnComplete.ExecuteIfBound(ErrorCode, Room);
        }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()