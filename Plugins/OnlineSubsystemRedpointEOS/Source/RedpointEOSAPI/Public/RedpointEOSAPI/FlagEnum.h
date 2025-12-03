// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

/**
 * A macro which defines the |, &, |= and &= operators for a scoped enumeration. These are all marked FORCEINLINE so
 * this macro should be used in the header where the enumeration is defined.
 *
 * Unfortunately there doesn't appear to be any way of using templates instead of a macro here:
 * - 'enum class' can not inherit from non-integral types i.e. if you make it inherit from a template, that template
 * must be of the form 'using ... = uint8;', in which case there's no way to constrain the specializations of the
 * operators to only scoped enumerations using that template (because a type alias is invisible to type traits).
 * - You can't define these in a class and then do 'using ::TFlagHelpers<FlagEnum>::operator|' because using
 * declarations can not refer to a class member.
 * - You can't do 'auto& operator| = TFlagHelpers<K>::...' because this declares a function pointer (for which
 * 'operator|' as a name is invalid).
 * - You can't define a template like 'template<typename E> class TFlagEnum { using enum E }' because using-enum can not
 * refer to a dependant type; this prevents the enumeration values of E from being available in TFlagEnum. If you could
 * do this, then you could do 'using K = TFlagEnum<enum class { ... }>' but it isn't permitted.
 *
 * This macro is defined in RedpointEOSAPI as we need to use it in RedpointEOSPlatform, which can't depend on
 * RedpointEOSCore.
 */
#define REDPOINT_EOS_DECLARE_FLAG_ENUMERATION_OPERATORS(FlagEnum)                                                      \
    FORCEINLINE FlagEnum operator|(const FlagEnum &A, const FlagEnum &B)                                               \
    {                                                                                                                  \
        using Storage = std::underlying_type<FlagEnum>::type;                                                          \
        return static_cast<FlagEnum>(static_cast<Storage>(A) | static_cast<Storage>(B));                               \
    }                                                                                                                  \
                                                                                                                       \
    FORCEINLINE FlagEnum operator&(const FlagEnum &A, const FlagEnum &B)                                               \
    {                                                                                                                  \
        using Storage = std::underlying_type<FlagEnum>::type;                                                          \
        return static_cast<FlagEnum>(static_cast<Storage>(A) & static_cast<Storage>(B));                               \
    }                                                                                                                  \
                                                                                                                       \
    FORCEINLINE FlagEnum &operator|=(FlagEnum &A, const FlagEnum &B)                                                   \
    {                                                                                                                  \
        using Storage = std::underlying_type<FlagEnum>::type;                                                          \
        return *reinterpret_cast<FlagEnum *>(&((*reinterpret_cast<Storage *>(&A)) |= static_cast<Storage>(B)));        \
    }                                                                                                                  \
                                                                                                                       \
    FORCEINLINE FlagEnum &operator&=(FlagEnum &A, const FlagEnum &B)                                                   \
    {                                                                                                                  \
        using Storage = std::underlying_type<FlagEnum>::type;                                                          \
        return *reinterpret_cast<FlagEnum *>(&((*reinterpret_cast<Storage *>(&A)) &= static_cast<Storage>(B)));        \
    }

REDPOINT_EOS_CODE_GUARD_END()