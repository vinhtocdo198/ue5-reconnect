// Copyright June Rhodes. All Rights Reserved.

#pragma once

// @note: When Unreal Engine 5.6 is supported, we will be dropping support for Unreal Engine 5.3, which is the last
// engine version to not have GetTypeHash for TOptional<> built-in. This code should still be removed when 5.6 is
// stable.
#if !REDPOINT_EOS_UE_5_4_OR_LATER

template <typename OptionalType>
inline auto GetTypeHash(const TOptional<OptionalType> &Optional) -> decltype(GetTypeHash(*Optional))
{
    return Optional.IsSet() ? GetTypeHash(*Optional) : 0;
}

#endif