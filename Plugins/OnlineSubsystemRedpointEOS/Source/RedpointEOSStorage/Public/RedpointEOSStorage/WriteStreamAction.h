// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3574945406, Redpoint::EOS::Storage)
{

enum class EWriteStreamAction : uint8
{
    /**
     * Continue writing more data.
     */
    Continue,

    /**
     * Complete writing the data (this is the last chunk of data to write).
     */
    Complete,

    /**
     * Indicate a failure in write data, and that the upload should stop.
     */
    Fail,

    /**
     * Stop writing the data, but not due to an error.
     */
    Cancel,
};

}

namespace Redpoint::EOS::Storage
{
REDPOINT_EOS_FILE_NS_EXPORT(3574945406, Redpoint::EOS::Storage, EWriteStreamAction)
}

REDPOINT_EOS_CODE_GUARD_END()