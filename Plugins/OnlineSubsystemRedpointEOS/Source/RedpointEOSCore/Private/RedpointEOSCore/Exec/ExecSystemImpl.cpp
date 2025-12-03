// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Exec/ExecSystemImpl.h"

#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Misc/CoreMisc.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1630525016, Redpoint::EOS::Core::Exec)
{
using namespace ::Redpoint::EOS::Core::Utils;

void FExecSystemImpl::RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &Entries)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    for (const auto &KV : this->RegisteredExecs)
    {
        auto Target = KV.Value.Pin();
        if (Target.IsValid())
        {
            TArray<FAutoCompleteCommand> TempEntries;
            Target->RegisterAutoCompleteEntries(TempEntries);
            for (auto &TempEntry : TempEntries)
            {
                TempEntry.Command = FString::Printf(TEXT("Redpoint %s %s"), *KV.Key, *TempEntry.Command);
                Entries.Add(TempEntry);
            }
        }
    }
}

void FExecSystemImpl::RegisterEvents()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    UConsole::RegisterConsoleAutoCompleteEntries.AddSP(this, &FExecSystemImpl::RegisterAutoCompleteEntries);
}

void FExecSystemImpl::Register(const FString &InName, const TSharedRef<FExecWithAutoComplete> &InExec)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    this->RegisteredExecs.Add(InName, InExec);

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

bool FExecSystemImpl::Exec(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    for (const auto &KV : this->RegisteredExecs)
    {
        auto Target = KV.Value.Pin();
        if (Target.IsValid())
        {
            if (FParse::Command(&Cmd, *KV.Key))
            {
                return Target->Exec(InWorld, Cmd, Ar);
            }
        }
    }
    return false;
}

static bool RedpointExec(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (FParse::Command(&Cmd, TEXT("Redpoint")))
    {
        // Try to find the platform instance.
        auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(InWorld);

        if (PlatformHandle.IsValid())
        {
            auto ExecSystem = StaticCastSharedRef<FExecSystemImpl>(PlatformHandle->GetSystem<IExecSystem>());
            return ExecSystem->Exec(InWorld, Cmd, Ar);
        }
    }

    return false;
}

static FStaticSelfRegisteringExec RedpointExecRegistration(&RedpointExec);

}

REDPOINT_EOS_CODE_GUARD_END()