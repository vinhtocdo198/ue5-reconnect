// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2757070145, Redpoint::EOS::Core::Utils)
{

class REDPOINTEOSCORE_API IHasEventRegistration
{
public:
    IHasEventRegistration() = default;
    UE_NONCOPYABLE(IHasEventRegistration);
    virtual ~IHasEventRegistration();

    virtual void RegisterEvents() = 0;
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(2757070145, Redpoint::EOS::Core::Utils, IHasEventRegistration)
}

REDPOINT_EOS_CODE_GUARD_END()