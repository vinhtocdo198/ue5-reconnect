// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PresenceModification
{

class REDPOINTEOSAPI_API FSetRawRichText
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        PresenceModification,
        SetRawRichText,
        EOS_PRESENCEMODIFICATION_SETRAWRICHTEXT_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> RichText;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()