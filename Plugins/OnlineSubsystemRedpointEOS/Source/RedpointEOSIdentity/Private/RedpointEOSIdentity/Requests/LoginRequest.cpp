// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/Requests/LoginRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1245715952, Redpoint::EOS::Identity::Requests)
{
using namespace ::Redpoint::EOS::Identity::Requests;

FLoginRequest::FLoginRequest(FIdentityUserSlot InUserSlot)
    : UserSlot(InUserSlot)
    , Type()
    , Id()
    , Token()
{
}

FLoginRequest::FLoginRequest(
    FIdentityUserSlot InUserSlot,
    const FString &InType,
    const FString &InId,
    const FString &InToken)
    : UserSlot(InUserSlot)
    , Type(InType)
    , Id(InId)
    , Token(InToken)
{
}

}

REDPOINT_EOS_CODE_GUARD_END()