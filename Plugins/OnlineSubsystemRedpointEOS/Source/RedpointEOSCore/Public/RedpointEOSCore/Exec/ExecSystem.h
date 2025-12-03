// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Exec/ExecWithAutoComplete.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1025384058, Redpoint::EOS::Core::Exec)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Exec;

class REDPOINTEOSCORE_API IExecSystem : public IPlatformInstanceSystem
{
public:
    static FName GetSystemName();
    static TSharedRef<IExecSystem> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

public:
    IExecSystem() = default;
    UE_NONCOPYABLE(IExecSystem);
    virtual ~IExecSystem() override = default;

    virtual void Register(const FString &InName, const TSharedRef<FExecWithAutoComplete> &InExec) = 0;
};

}

namespace Redpoint::EOS::Core::Exec
{
REDPOINT_EOS_FILE_NS_EXPORT(1025384058, Redpoint::EOS::Core::Exec, IExecSystem)
}

REDPOINT_EOS_CODE_GUARD_END()