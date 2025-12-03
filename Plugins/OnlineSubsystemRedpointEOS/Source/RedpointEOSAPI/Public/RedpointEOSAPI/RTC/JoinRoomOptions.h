// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTC
{

namespace EJoinRoomOptions
{

enum Type : uint32
{
    None = 0,
    EnableEcho = 1,
};

}

}

REDPOINT_EOS_CODE_GUARD_END()