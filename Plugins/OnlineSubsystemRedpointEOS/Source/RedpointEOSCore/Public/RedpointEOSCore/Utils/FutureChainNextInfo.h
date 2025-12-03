// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Async/Future.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/FunctionTraits.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1409873568, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::API;

template <typename Func> struct TFutureChainNextInfo;

template <typename T> struct TFutureChainNextInfo : public TFutureChainNextInfo<decltype(&T::operator())>
{
};

template <typename NextType, typename PreviousType> struct TFutureChainNextInfo<TFuture<NextType>(PreviousType)>
{
    using Next = NextType;
    using Previous = PreviousType;
};

REDPOINT_EOS_EXPANSIVE_FUNCTION_TRAIT_TEMPLATE(TFutureChainNextInfo);

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(1409873568, Redpoint::EOS::Core::Utils, TFutureChainNextInfo);
}

REDPOINT_EOS_CODE_GUARD_END()
