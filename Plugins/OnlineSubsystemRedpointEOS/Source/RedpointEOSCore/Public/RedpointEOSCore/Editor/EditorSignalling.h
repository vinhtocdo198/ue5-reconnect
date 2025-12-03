// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(683639614, Redpoint::EOS::Core::Editor)
{

#if WITH_EDITOR

class REDPOINTEOSCORE_API FEditorSignalling
{
public:
    typedef TMulticastDelegate<void(const FString &Context, const FString &SignalId)> FOnEditorCustomSignal;

private:
    static FOnEditorCustomSignal OnEditorCustomSignalValue;

public:
    static FOnEditorCustomSignal &OnEditorCustomSignal();
};

#endif

}

namespace Redpoint::EOS::Core::Editor
{
#if WITH_EDITOR
REDPOINT_EOS_FILE_NS_EXPORT(683639614, Redpoint::EOS::Core::Editor, FEditorSignalling)
#endif
}

REDPOINT_EOS_CODE_GUARD_END()