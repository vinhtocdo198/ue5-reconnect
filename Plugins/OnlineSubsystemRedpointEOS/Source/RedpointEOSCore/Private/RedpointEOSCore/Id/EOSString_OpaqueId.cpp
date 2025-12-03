// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Id/EOSString_OpaqueId.h"

// @note: This is considered legacy code, ported from OnlineSubsystemRedpointEOS so that FUniqueNetIdEOS can live in
// this module. All API calls outside FUniqueNetIdEOS use RedpointEOSAPI which handles the string marshalling in a more
// modern way.

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2619528734, Redpoint::EOS::Core::Id)
{

template <> const EOS_ProductUserId EOSString_ProductUserId::None = nullptr;
char const EOS_ProductUserId_TypeName[] = "EOS_ProductUserId";

template <> const EOS_EpicAccountId EOSString_EpicAccountId::None = nullptr;
char const EOS_EpicAccountId_TypeName[] = "EOS_EpicAccountId";

}

REDPOINT_EOS_CODE_GUARD_END()