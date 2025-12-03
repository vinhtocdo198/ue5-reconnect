// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Async/Future.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/FunctionTraits.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3950885332, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::API;

template <typename Func> struct TFutureChainThenInfo;

template <typename T> struct TFutureChainThenInfo : public TFutureChainThenInfo<decltype(&T::operator())>
{
};

template <typename NextType, typename PreviousType>
struct TFutureChainThenInfo<TFuture<NextType>(TFuture<PreviousType>)>
{
    using Next = NextType;
    using Previous = PreviousType;
};

REDPOINT_EOS_EXPANSIVE_FUNCTION_TRAIT_TEMPLATE(TFutureChainThenInfo);

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(3950885332, Redpoint::EOS::Core::Utils, TFutureChainThenInfo);
}

REDPOINT_EOS_CODE_GUARD_END()
