// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Exec/ExecSystem.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1630525016, Redpoint::EOS::Core::Exec)
{
using namespace ::Redpoint::EOS::Core::Exec;
using namespace ::Redpoint::EOS::Core::Utils;

class FExecSystemImpl : public IExecSystem, public IHasEventRegistration, public TSharedFromThis<FExecSystemImpl>
{
private:
    TMap<FString, TWeakPtr<FExecWithAutoComplete>> RegisteredExecs;

    void RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &);

public:
    FExecSystemImpl() = default;
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FExecSystemImpl);
    virtual ~FExecSystemImpl() override = default;

    virtual void Register(const FString &InName, const TSharedRef<FExecWithAutoComplete> &InExec) override;

    bool Exec(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar);
};

}

namespace Redpoint::EOS::Core::Exec
{
REDPOINT_EOS_FILE_NS_EXPORT(1630525016, Redpoint::EOS::Core::Exec, FExecSystemImpl)
}

REDPOINT_EOS_CODE_GUARD_END()