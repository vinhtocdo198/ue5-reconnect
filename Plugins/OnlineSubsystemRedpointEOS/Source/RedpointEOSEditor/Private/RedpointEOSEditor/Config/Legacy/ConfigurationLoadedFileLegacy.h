// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSEditor/Config/ConfigurationWriterPendingChanges.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(4146539377, Redpoint::EOS::Editor::Config::Legacy)
{
using namespace ::Redpoint::EOS::Editor::Config;

/**
 * It's not safe to store FConfigFile* pointers for any length of time, so this value should only
 * ever be temporarily on the stack while doing the write and then released.
 */
struct FConfigurationLoadedFileLegacy
{
public:
    FConfigurationLoadedFileLegacy(
        FConfigFile *InF,
        FConfigFile *InSF,
        const TSharedRef<FConfigurationWriterPendingChanges> &InInfo);
    FConfigFile *F;
    FConfigFile *SF;
    TSharedRef<FConfigurationWriterPendingChanges> Info;
};

}

namespace Redpoint::EOS::Editor::Config::Legacy
{
REDPOINT_EOS_FILE_NS_EXPORT(4146539377, Redpoint::EOS::Editor::Config::Legacy, FConfigurationLoadedFileLegacy);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
