// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

template <typename Accessor, typename Accessor::Member Member> struct TRedpointAccessPrivate
{
    friend typename Accessor::Member GetPrivate(Accessor InAccessor)
    {
        return Member;
    }
};

REDPOINT_EOS_CODE_GUARD_END()