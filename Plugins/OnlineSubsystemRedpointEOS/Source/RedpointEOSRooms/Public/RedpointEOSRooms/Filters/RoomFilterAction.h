// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3971484324, Redpoint::EOS::Rooms::Filters)
{

enum class ERoomFilterAction : uint8
{
    /**
     * Continue passing this action onto further filters, or handle the request if this is the last action. The filter
     * must not invoke the completion delegate.
     */
    Continue,

    /**
     * Do not continue passing the request through filters. The filter must ensure the completion delegate is invoked.
     */
    Terminate,
};

}

namespace Redpoint::EOS::Rooms::Filters
{
REDPOINT_EOS_FILE_NS_EXPORT(3971484324, Redpoint::EOS::Rooms::Filters, ERoomFilterAction)
}

REDPOINT_EOS_CODE_GUARD_END()