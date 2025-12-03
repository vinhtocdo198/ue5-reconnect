// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if 0

#include "RedpointEOSBuild/BuildEnvironment.h"
#include <array>

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::Util::ApiCallName
{

template <std::size_t N> using Type = std::array<TCHAR, N>;

template <std::size_t N> constexpr Type<N> Constant(const TCHAR (&a)[N])
{
    Type<N> res{};
    for (std::size_t i = 0; i != N - 1; ++i)
    {
        res[i] = a[i];
    }
    return res;
}

template <std::size_t... Ns> constexpr Type<(Ns + ...)> Concat(const Type<Ns> &...as)
{
    Type<(Ns + ...)> res{};
    std::size_t i = 0;
    auto l = [&](const auto &a) {
        for (auto c : a)
        {
            if (c != '\0')
            {
                res[i++] = c;
            }
        }
    };
    (l(as), ...);
    for (; i < (Ns + ...); i++)
    {
        res[i] = '\0';
    }
    return res;
}

}

REDPOINT_EOS_CODE_GUARD_END()

#endif