// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Legacy/ConfigurationLoadedFileLegacy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(4146539377, Redpoint::EOS::Editor::Config::Legacy)
{
using namespace ::Redpoint::EOS::Editor::Config;

FConfigurationLoadedFileLegacy::FConfigurationLoadedFileLegacy(
    FConfigFile *InF,
    FConfigFile *InSF,
    const TSharedRef<FConfigurationWriterPendingChanges> &InInfo)
    : F(InF)
    , SF(InSF)
    , Info(InInfo)
{
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
