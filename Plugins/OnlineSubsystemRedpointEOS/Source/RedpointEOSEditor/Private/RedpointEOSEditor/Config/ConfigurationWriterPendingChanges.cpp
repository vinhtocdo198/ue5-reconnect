// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/ConfigurationWriterPendingChanges.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2242189668, Redpoint::EOS::Editor::Config)
{

#if REDPOINT_EOS_UE_5_5_OR_LATER

FConfigurationWriterPendingChanges::FConfigurationWriterPendingChanges(
    const TTuple<FString, EConfigurationFileType, FName> &InitFromMap)
    : Path(InitFromMap.Get<0>())
    , File(InitFromMap.Get<1>())
    , Platform(InitFromMap.Get<2>())
    , Removals()
    , Upserts()
    , Replacements()
    , bIsModified(false)
{
}

#else

FConfigurationWriterPendingChanges::FConfigurationWriterPendingChanges(const FString &InPath)
    : Path(InPath)
    , Removals()
    , Upserts()
    , Replacements()
    , bIsModified(false)
{
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()
