// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/SessionDetails/CopyInfo.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionDetails
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_IMPL(CopyInfo);

void FCopyInfo::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
}

void FCopyInfo::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    auto &SessionDetails = Result;
    const auto &NativeSessionDetails = NativeResult;

    SessionDetails.SessionId = Converter.FromAnsi(NativeSessionDetails.SessionId);
    SessionDetails.HostAddress = Converter.FromUtf8(NativeSessionDetails.HostAddress);
    SessionDetails.NumOpenPublicConnections = NativeSessionDetails.NumOpenPublicConnections;
    SessionDetails.OwnerUserId = NativeSessionDetails.OwnerUserId;
    SessionDetails.OwnerServerClientId = Converter.FromAnsi(NativeSessionDetails.OwnerServerClientId);
    SessionDetails.BucketId = Converter.FromAnsi(NativeSessionDetails.Settings->BucketId);
    SessionDetails.NumPublicConnections = NativeSessionDetails.Settings->NumPublicConnections;
    SessionDetails.bAllowJoinInProgress = NativeSessionDetails.Settings->bAllowJoinInProgress == EOS_TRUE;
    SessionDetails.PermissionLevel = NativeSessionDetails.Settings->PermissionLevel;
    SessionDetails.bInvitesAllowed = NativeSessionDetails.Settings->bInvitesAllowed == EOS_TRUE;
    SessionDetails.bSanctionsEnabled = NativeSessionDetails.Settings->bSanctionsEnabled == EOS_TRUE;
    if (NativeSessionDetails.Settings->AllowedPlatformIds == nullptr ||
        NativeSessionDetails.Settings->AllowedPlatformIdsCount == 0)
    {
        SessionDetails.AllowedPlatformIds = TArray<Common::EOnlinePlatformType>();
    }
    else
    {
        for (uint32 i = 0; i < NativeSessionDetails.Settings->AllowedPlatformIdsCount; i++)
        {
            SessionDetails.AllowedPlatformIds.Add(
                (Common::EOnlinePlatformType)NativeSessionDetails.Settings->AllowedPlatformIds[i]);
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()