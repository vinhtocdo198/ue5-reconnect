// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(523341298, Redpoint::EOS::Editor::Config)
{

enum class EConfigurationWriterFlushResult : uint8
{
    // The changes were successfully flushed to disk.
    Success,

    // One or more configuration files could not be checked out from
    // source control, so the changes could not be saved.
    FailedToCheckOutFiles,
};

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(523341298, Redpoint::EOS::Editor::Config, EConfigurationWriterFlushResult);
}

REDPOINT_EOS_CODE_GUARD_END()
