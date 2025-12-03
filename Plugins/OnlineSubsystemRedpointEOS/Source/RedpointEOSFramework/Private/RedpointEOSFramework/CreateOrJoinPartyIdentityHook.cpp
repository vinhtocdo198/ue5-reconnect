// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFramework/CreateOrJoinPartyIdentityHook.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "GameMapsSettings.h"
#include "LogRedpointEOSFramework.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSFramework/CreateAutomaticParty.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureSearchState.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureSearchState.h"
#include "RedpointEOSRooms/RoomSystem.h"
#include "RedpointFrameworkSubsystem.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2146425322, Redpoint::EOS::Framework)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Framework;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Rooms::Requests;

void FCreateOrJoinPartyIdentityHook::OnStartSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStartSystemComplete &InOnComplete)
{
    auto RoomSystem = InContext.PlatformHandle->GetSystem<IRoomSystem>();

    bool bEnsureAlwaysInParty = false;
    int32 PartyMaximumSize = 4;
    bool bEnableVoiceChatInParty = false;
    GetAutomaticPartySettings(
        InContext.PlatformHandle,
        bEnsureAlwaysInParty,
        PartyMaximumSize,
        bEnableVoiceChatInParty);

    if (!bEnsureAlwaysInParty)
    {
        UE_LOG(
            LogRedpointEOSFramework,
            Verbose,
            TEXT("The game does not 'Ensure Always in Party' turned on in the Project Settings. No party will be "
                 "created or joined during login."));
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    const auto CreateParty = [RoomSystem, InUser, InOnComplete, PartyMaximumSize, bEnableVoiceChatInParty]() {
        CreateAutomaticParty(
            PartyMaximumSize,
            bEnableVoiceChatInParty,
            RoomSystem,
            InUser->GetUserId(),
            IRoomSystem::FOnCreateRoomComplete::CreateLambda(
                [InOnComplete](const FError &ErrorCode, const FRoomPtr &Room) {
                    if (!ErrorCode.WasSuccessful())
                    {
                        UE_LOG(
                            LogRedpointEOSFramework,
                            Error,
                            TEXT("Unable to create automatic presence-enabled party on login: %s"),
                            *ErrorCode.ToLogString());
                    }
                    InOnComplete.ExecuteIfBound(ErrorCode);
                }));
    };

    for (const auto &Invite : RoomSystem->GetCurrentRoomInvites(InUser->GetUserId(), NAME_Game))
    {
        auto SearchResult = Invite->GetRoomInviteSearchResult();
        auto PartyTypeFeature = SearchResult->GetRoomFeatureSearchState<IPartyTypeRoomFeatureSearchState>();
        auto PresenceFeature = SearchResult->GetRoomFeatureSearchState<IPresenceRoomFeatureSearchState>();

        if (PresenceFeature.IsValid() && PartyTypeFeature.IsValid() &&
            PartyTypeFeature->PartyTypeId() == PartyTypePrimaryPartyId)
        {
            // This is an invite to a party. Accept this one.
            UE_LOG(
                LogRedpointEOSFramework,
                Verbose,
                TEXT("Accepting invite to party with room ID '%s'..."),
                *Invite->GetRoomInviteSearchResult()->GetRoomId()->ToString());
            RoomSystem->JoinRoom(
                FJoinRoomRequest(ERoomScope::User, Invite),
                IRoomSystem::FOnJoinRoomComplete::CreateLambda(
                    [InOnComplete, CreateParty](const FError &ErrorCode, const FRoomPtr &Room) {
                        if (ErrorCode.WasSuccessful())
                        {
                            UE_LOG(
                                LogRedpointEOSFramework,
                                Verbose,
                                TEXT("Successfully joined existing party invite on game startup!"));
                            InOnComplete.ExecuteIfBound(ErrorCode);
                        }
                        else
                        {
                            UE_LOG(
                                LogRedpointEOSFramework,
                                Error,
                                TEXT("Failed to join existing party invite, falling back to creating a new party: %s"),
                                *ErrorCode.ToLogString());
                            CreateParty();
                        }
                    }));
            break;
        }
    }

    UE_LOG(
        LogRedpointEOSFramework,
        Verbose,
        TEXT("No existing party invite found, falling back to creating a new party..."));
    CreateParty();
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()