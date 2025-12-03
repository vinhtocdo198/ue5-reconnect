// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/LobbyDetails/CopyInfo.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyDetails
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_IMPL(CopyInfo);

void FCopyInfo::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

void FCopyInfo::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
    Result.LobbyOwnerUserId = NativeResult.LobbyOwnerUserId;
    Result.PermissionLevel = NativeResult.PermissionLevel;
    Result.AvailableSlots = NativeResult.AvailableSlots;
    Result.MaxMembers = NativeResult.MaxMembers;
    Result.bAllowInvites = NativeResult.bAllowInvites == EOS_TRUE ? true : false;
    Result.BucketId = Converter.FromAnsi(NativeResult.BucketId);
    Result.bAllowHostMigration = NativeResult.bAllowHostMigration == EOS_TRUE ? true : false;
    Result.bRTCRoomEnabled = NativeResult.bRTCRoomEnabled == EOS_TRUE ? true : false;
    Result.bAllowJoinById = NativeResult.bAllowJoinById == EOS_TRUE ? true : false;
    Result.bRejoinAfterKickRequiresInvite = NativeResult.bRejoinAfterKickRequiresInvite == EOS_TRUE ? true : false;
    Result.bPresenceEnabled = NativeResult.bPresenceEnabled == EOS_TRUE ? true : false;
    if (NativeResult.AllowedPlatformIds == nullptr || NativeResult.AllowedPlatformIdsCount == 0)
    {
        Result.AllowedPlatformIds = TArray<Common::EOnlinePlatformType>();
    }
    else
    {
        for (uint32 i = 0; i < NativeResult.AllowedPlatformIdsCount; i++)
        {
            Result.AllowedPlatformIds.Add((Common::EOnlinePlatformType)NativeResult.AllowedPlatformIds[i]);
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()