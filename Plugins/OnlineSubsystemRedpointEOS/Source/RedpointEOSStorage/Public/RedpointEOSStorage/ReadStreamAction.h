// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3994585470, Redpoint::EOS::Storage)
{

enum class EReadStreamAction : uint8
{
    /**
     * Continue reading the data.
     */
    Continue,

    /**
     * Indicate a failure in reading data, and that the download should stop.
     */
    Fail,

    /**
     * Stop reading the data, but not due to an error.
     */
    Cancel,
};

}

namespace Redpoint::EOS::Storage
{
REDPOINT_EOS_FILE_NS_EXPORT(3994585470, Redpoint::EOS::Storage, EReadStreamAction)
}

REDPOINT_EOS_CODE_GUARD_END()