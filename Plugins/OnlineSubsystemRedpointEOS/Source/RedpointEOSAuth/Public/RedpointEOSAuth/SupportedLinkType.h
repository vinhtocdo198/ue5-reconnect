// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2890172839, Redpoint::EOS::Auth)
{

enum class ESupportedLinkType : uint8
{
    None = 1,
    PersistentAuth = 2,
    ExternalAuth = 3
};

}

namespace Redpoint::EOS::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(2890172839, Redpoint::EOS::Auth, ESupportedLinkType)
}

REDPOINT_EOS_CODE_GUARD_END()