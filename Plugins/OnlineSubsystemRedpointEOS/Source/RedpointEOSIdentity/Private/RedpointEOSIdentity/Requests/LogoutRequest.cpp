// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/Requests/LogoutRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4051209205, Redpoint::EOS::Identity::Requests)
{
using namespace ::Redpoint::EOS::Identity::Requests;

FLogoutRequest::FLogoutRequest(FIdentityUserSlot InUserSlot)
    : UserSlot(InUserSlot)
{
}

}

REDPOINT_EOS_CODE_GUARD_END()