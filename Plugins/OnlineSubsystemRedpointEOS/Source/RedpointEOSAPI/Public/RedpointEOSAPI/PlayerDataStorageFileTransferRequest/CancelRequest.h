// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/SyncNoOptions.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PlayerDataStorageFileTransferRequest
{

class REDPOINTEOSAPI_API FCancelRequest
{
    REDPOINT_EOSSDK_API_CALL_SYNC_NO_OPTIONS_BEGIN(PlayerDataStorageFileTransferRequest, CancelRequest)

    REDPOINT_EOSSDK_API_CALL_SYNC_NO_OPTIONS_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()