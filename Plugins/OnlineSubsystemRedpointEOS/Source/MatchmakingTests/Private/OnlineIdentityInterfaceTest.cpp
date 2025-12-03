// Copyright June Rhodes. All Rights Reserved.

#include "OnlineIdentityInterfaceTest.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "TestHelpers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<const FUniqueNetId> FOnlineIdentityInterfaceTest::CreateUniquePlayerId(uint8 *Bytes, int32 Size)
{
    return MakeShared<FUniqueNetIdMMTest>(*reinterpret_cast<int32 *>(Bytes));
}

#endif

REDPOINT_EOS_CODE_GUARD_END()