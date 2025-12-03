// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/PresenceModification/SetData.h"

#include "RedpointEOSAPI/Templates/Impl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PresenceModification
{

REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_IMPL(SetData);

void FSetData::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    TArray<EOS_Presence_DataRecord> Records;
    for (const auto &KV : *Options.Records)
    {
        EOS_Presence_DataRecord Record = {};
        Record.ApiVersion = EOS_PRESENCE_DATARECORD_API_LATEST;
        Record.Key = Allocator.AsAnsi(*KV.Key);
        Record.Value = Allocator.AsAnsi(*KV.Value);
        Records.Add(Record);
    }
    uint32_t RecordsCount;
    NativeOptions.Records = Allocator.AllocateArray(Records, RecordsCount);
    NativeOptions.RecordsCount = static_cast<int32_t>(RecordsCount);
}

}

REDPOINT_EOS_CODE_GUARD_END()