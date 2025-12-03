// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFramework/CreatePartyOnLeaveRoomFilter.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "LogRedpointEOSFramework.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSFramework/CreateAutomaticParty.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeatureRequest.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3198079518, Redpoint::EOS::Framework)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Framework;
using namespace ::Redpoint::EOS::Rooms::Features;

ERoomFilterAction FCreatePartyOnLeaveRoomFilter::OnLeaveRoom(
    const FPlatformHandle &PlatformHandle,
    const FRoomRef &Room,
    FLeaveRoomRequest &Request,
    const IRoomSystem::FOnLeaveRoomComplete &OnComplete)
{
    bool bEnsureAlwaysInParty = false;
    int32 PartyMaximumSize = 4;
    bool bEnableVoiceChatInParty = false;
    GetAutomaticPartySettings(PlatformHandle, bEnsureAlwaysInParty, PartyMaximumSize, bEnableVoiceChatInParty);

    if (!bEnsureAlwaysInParty)
    {
        // We're not ensuring parties are always present.
        return ERoomFilterAction::Continue;
    }

    if (Room->HasRoomFeature<IPartyTypeRoomFeature>() &&
        Room->GetRoomFeatureChecked<IPartyTypeRoomFeature>()->GetPartyTypeId() == PartyTypePrimaryPartyId)
    {
        // We're leaving the primary party. Redirect it back to a create call.
        CreateAutomaticParty(
            PartyMaximumSize,
            bEnableVoiceChatInParty,
            PlatformHandle->GetSystem<IRoomSystem>(),
            Room->GetBoundForLocalUser(),
            IRoomSystem::FOnCreateRoomComplete::CreateLambda([OnComplete](const FError &ErrorCode, const FRoomPtr &) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogRedpointEOSFramework,
                        Error,
                        TEXT("Unable to create automatic presence-enabled party in response to room leave: %s"),
                        *ErrorCode.ToLogString());
                }
                OnComplete.ExecuteIfBound(ErrorCode);
            }));
        return ERoomFilterAction::Terminate;
    }

    // Let this be handled normally.
    return ERoomFilterAction::Continue;
}

void FCreatePartyOnLeaveRoomFilter::OnRoomLeftExternally(
    const FPlatformHandle &PlatformHandle,
    const FRoomRef &Room,
    ERoomLeaveReason RoomLeaveReason)
{
    bool bEnsureAlwaysInParty = false;
    int32 PartyMaximumSize = 4;
    bool bEnableVoiceChatInParty = false;
    GetAutomaticPartySettings(PlatformHandle, bEnsureAlwaysInParty, PartyMaximumSize, bEnableVoiceChatInParty);

    if (!bEnsureAlwaysInParty)
    {
        // We're not ensuring parties are always present.
        return;
    }

    if (Room->HasRoomFeature<IPartyTypeRoomFeature>() &&
        Room->GetRoomFeatureChecked<IPartyTypeRoomFeature>()->GetPartyTypeId() == PartyTypePrimaryPartyId)
    {
        // We've left the primary party. Start creating a new primary party in response.
        CreateAutomaticParty(
            PartyMaximumSize,
            bEnableVoiceChatInParty,
            PlatformHandle->GetSystem<IRoomSystem>(),
            Room->GetBoundForLocalUser(),
            IRoomSystem::FOnCreateRoomComplete::CreateLambda([](const FError &ErrorCode, const FRoomPtr &) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogRedpointEOSFramework,
                        Error,
                        TEXT("Unable to create automatic presence-enabled party in response to room leave: %s"),
                        *ErrorCode.ToLogString());
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSFramework,
                        Verbose,
                        TEXT("Successfully created automatic presence-enabled party in response to unexpected room "
                             "leave."));
                }
            }));
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()