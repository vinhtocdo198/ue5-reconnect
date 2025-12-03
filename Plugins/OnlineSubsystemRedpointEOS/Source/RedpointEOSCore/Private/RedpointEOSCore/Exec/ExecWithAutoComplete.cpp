// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Exec/ExecWithAutoComplete.h"

#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1677745012, Redpoint::EOS::Core::Exec)
{

void FExecWithAutoComplete::RebuildAutoCompleteEntries()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (GEngine != nullptr)
    {
        UConsole *ViewportConsole =
            (GEngine->GameViewport != nullptr) ? GEngine->GameViewport->ViewportConsole : nullptr;
        if (ViewportConsole != nullptr)
        {
            ViewportConsole->BuildRuntimeAutoCompleteList(true);
        }
    }
}

void FExecWithAutoComplete::RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &)
{
}

}

REDPOINT_EOS_CODE_GUARD_END()