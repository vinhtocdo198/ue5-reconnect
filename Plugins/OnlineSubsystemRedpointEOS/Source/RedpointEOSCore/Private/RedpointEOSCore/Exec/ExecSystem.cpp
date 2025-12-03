// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Exec/ExecSystem.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Exec/ExecSystemImpl.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1025384058, Redpoint::EOS::Core::Exec)
{
using namespace ::Redpoint::EOS::Core::Exec;
using namespace ::Redpoint::EOS::Core::Utils;

FName IExecSystem::GetSystemName()
{
    return FName(TEXT("ExecSystem"));
}

TSharedRef<IExecSystem> IExecSystem::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    return MakeSharedWithEvents<FExecSystemImpl>();
}

}

REDPOINT_EOS_CODE_GUARD_END()