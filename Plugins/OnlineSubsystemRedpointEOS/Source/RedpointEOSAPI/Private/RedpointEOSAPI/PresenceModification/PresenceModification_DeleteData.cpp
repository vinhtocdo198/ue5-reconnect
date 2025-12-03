// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/PresenceModification/DeleteData.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PresenceModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(DeleteData);

void FDeleteData::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    TArray<EOS_PresenceModification_DataRecordId> RecordIds;
    for (const auto &Key : *Options.RecordIds)
    {
        EOS_PresenceModification_DataRecordId RecordId = {};
        RecordId.ApiVersion = EOS_PRESENCEMODIFICATION_DATARECORDID_API_LATEST;
        RecordId.Key = Allocator.AsAnsi(*Key);
        RecordIds.Add(RecordId);
    }
    uint32_t RecordsCount;
    NativeOptions.Records = Allocator.AllocateArray(RecordIds, RecordsCount);
    NativeOptions.RecordsCount = static_cast<int32_t>(RecordsCount);
}

}

REDPOINT_EOS_CODE_GUARD_END()