// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1758170589, Redpoint::EOS::Storage)
{

enum class EStorageType : uint8
{
    /**
     * Per-player storage that can be read and written to.
     */
    PlayerDataStorage,

    /**
     * Title storage that can only be read. To upload content to it, use the Epic Online Services Developer Portal.
     */
    TitleStorage,
};

}

namespace Redpoint::EOS::Storage
{
REDPOINT_EOS_FILE_NS_EXPORT(1758170589, Redpoint::EOS::Storage, EStorageType)
}

REDPOINT_EOS_CODE_GUARD_END()