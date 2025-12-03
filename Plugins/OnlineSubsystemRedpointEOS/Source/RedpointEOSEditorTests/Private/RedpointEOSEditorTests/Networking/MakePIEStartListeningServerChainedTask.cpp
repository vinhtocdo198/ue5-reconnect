// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditorTests/Networking/MakePIEStartListeningServerChainedTask.h"

#include "UnrealEngine.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1931797628, Redpoint::EOS::Editor::Tests::Networking)
{

FMakePIEStartListeningServerChainedTask::FMakePIEStartListeningServerChainedTask(
    int InInstanceIndex,
    uint16 InPort,
    const FString &InExtraParams)
    : InstanceIndex(InInstanceIndex)
    , Port(InPort)
    , ExtraParams(InExtraParams)
{
}

void FMakePIEStartListeningServerChainedTask::ExecuteAsync(
    const FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    for (const FWorldContext &WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World() != nullptr &&
            WorldContext.PIEInstance == this->InstanceIndex)
        {
            FURL URL(WorldContext.LastURL);
            URL.Port = this->Port;
            URL.AddOption(TEXT("listen"));
            if (!this->ExtraParams.IsEmpty())
            {
                URL.AddOption(*this->ExtraParams);
            }
            WorldContext.World()->Listen(URL);
            OnDone.ExecuteIfBound(true);
            return;
        }
    }

    Context->Assert().TestTrue(FString::Printf(TEXT("Expected PIE instance %d to exist"), this->InstanceIndex), false);
    OnDone.ExecuteIfBound(false);
    return;
}

}

REDPOINT_EOS_CODE_GUARD_END()