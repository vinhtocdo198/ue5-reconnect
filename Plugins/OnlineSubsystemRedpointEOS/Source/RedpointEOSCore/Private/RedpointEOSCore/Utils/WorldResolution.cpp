// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Utils/WorldResolution.h"

#include "Engine/Engine.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/InstancePool.h"
#include "RedpointEOSCore/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4062683160, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::Core;

TSoftObjectPtr<UWorld> FWorldResolution::GetWorld(const FName &InInstanceName, bool bAllowFailure)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    if (GEngine == nullptr)
    {
        if (!bAllowFailure)
        {
            UE_LOG(
                LogRedpointEOSCore,
                Error,
                TEXT("FWorldResolution::GetWorld: Called before the engine has initialized the first world."));
        }

        return nullptr;
    }

    TSoftObjectPtr<UWorld> WorldPtr = nullptr;
    if (InInstanceName.ToString() == TEXT("DefaultInstance"))
    {
        WorldPtr = GEngine->GetWorld();
        if (WorldPtr == nullptr)
        {
            // Okay, try this is a backup... seems to only be a thing outside of the editor.
            for (const auto &WorldContext : GEngine->GetWorldContexts())
            {
                WorldPtr = WorldContext.World();
                if (WorldPtr != nullptr)
                {
                    break;
                }
            }
        }
    }
    else
    {
        FWorldContext *WorldContext = GEngine->GetWorldContextFromHandle(InInstanceName);
        if (WorldContext == nullptr)
        {
            if (!bAllowFailure)
            {
                UE_LOG(
                    LogRedpointEOSCore,
                    Error,
                    TEXT("FWorldResolution::GetWorld: The world context handle is invalid."));
            }
            return nullptr;
        }

        WorldPtr = WorldContext->World();
    }

    if (!WorldPtr.IsValid())
    {
        if (!bAllowFailure)
        {
            if (InInstanceName.ToString() == TEXT("DefaultInstance"))
            {
                UE_LOG(
                    LogRedpointEOSCore,
                    Error,
                    TEXT("FWorldResolution::GetWorld: There is no active world."),
                    *InInstanceName.ToString());
            }
            else
            {
                UE_LOG(
                    LogRedpointEOSCore,
                    Error,
                    TEXT("FWorldResolution::GetWorld: There is no active world for world context '%s'."),
                    *InInstanceName.ToString());
            }
        }

        return nullptr;
    }

    return WorldPtr;
}

TSharedPtr<FPlatformInstance> FWorldResolution::TryGetPlatformHandle(const UWorld *InWorld)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    auto *WorldContext = GEngine->GetWorldContextFromWorld(InWorld);
    if (WorldContext == nullptr)
    {
        return nullptr;
    }
    auto InstanceName =
        WorldContext->WorldType == EWorldType::PIE ? WorldContext->ContextHandle : FName(TEXT("DefaultInstance"));
    auto PlatformHandle = IInstancePool::Pool().TryGetExisting(InstanceName);
    return PlatformHandle;
}

FPlatformHandle FWorldResolution::GetPlatformHandle(const UWorld *InWorld)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(InWorld);
    checkf(
        PlatformHandle.IsValid(),
        TEXT("Unable to locate platform handle when calling FWorldResolution::GetPlatformHandle."));
    return PlatformHandle.ToSharedRef();
}

}

REDPOINT_EOS_CODE_GUARD_END()