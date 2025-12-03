// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(338481362, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::Core::Utils;

template <typename InObjectType, typename... InArgTypes>
TSharedRef<InObjectType> MakeSharedWithEvents(InArgTypes &&...Args)
{
    static_assert(
        std::is_base_of<IHasEventRegistration, InObjectType>::value,
        "Expected InObjectType to implement IHasEventRegistration.");
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wredpoint.games/make-shared-with-events"
#endif
    auto Instance = MakeShared<InObjectType>(Forward<InArgTypes>(Args)...);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
    Instance->RegisterEvents();
    return Instance;
}

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(338481362, Redpoint::EOS::Core::Utils, MakeSharedWithEvents)
}

REDPOINT_EOS_CODE_GUARD_END()