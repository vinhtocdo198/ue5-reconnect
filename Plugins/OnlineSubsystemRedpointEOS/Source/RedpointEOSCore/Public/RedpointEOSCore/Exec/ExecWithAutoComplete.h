// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "ConsoleSettings.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1677745012, Redpoint::EOS::Core::Exec)
{

class REDPOINTEOSCORE_API FExecWithAutoComplete : public FExec
{
private:
    TMap<FString, TWeakPtr<FExec>> RegisteredExecs;

protected:
    void RebuildAutoCompleteEntries();

public:
    FExecWithAutoComplete() = default;
    UE_NONCOPYABLE(FExecWithAutoComplete);
    virtual ~FExecWithAutoComplete() override = default;

    virtual void RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &);
};

}

namespace Redpoint::EOS::Core::Exec
{
REDPOINT_EOS_FILE_NS_EXPORT(1677745012, Redpoint::EOS::Core::Exec, FExecWithAutoComplete)
}

REDPOINT_EOS_CODE_GUARD_END()