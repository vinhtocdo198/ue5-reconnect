// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/RTC/SetSetting.h"

#include "RedpointEOSAPI/Templates/Impl/Sync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTC
{

REDPOINT_EOSSDK_API_CALL_SYNC_IMPL(SetSetting);

void FSetSetting::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.SettingName = Allocator.AsAnsi(Options.SettingName);
    NativeOptions.SettingValue = Allocator.AsAnsi(Options.SettingValue);
}

}

REDPOINT_EOS_CODE_GUARD_END()