// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAntiCheat/AntiCheatSystem.h"

#include "RedpointEOSAntiCheat/NullAntiCheatSystemImpl.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Exec/ExecSystem.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"

#if WITH_SERVER_CODE
#include "RedpointEOSAntiCheat/DedicatedServerAntiCheatSystemImpl.h"
#endif
#if WITH_CLIENT_CODE
#include "RedpointEOSAntiCheat/GameAntiCheatSystemImpl.h"
#endif
#if WITH_EDITOR
#include "RedpointEOSAntiCheat/EditorAntiCheatSystemImpl.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(353349712, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Exec;
using namespace ::Redpoint::EOS::Core::Utils;

FName IAntiCheatSystem::GetSystemName()
{
    return FName(TEXT("IAntiCheatSystem"));
}

TSharedRef<IAntiCheatSystem> IAntiCheatSystem::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    if (InPlatformHandle->GetEnvironment()->Config->GetEnableAntiCheat())
    {
#if WITH_EDITOR
        // We're running in the editor, so we use an implementation that emulates Anti-Cheat calls.
        auto AntiCheat = MakeShared<FEditorAntiCheatSystemImpl>();
        InPlatformHandle->GetSystem<IExecSystem>()->Register(TEXT("AntiCheat"), AntiCheat);
        return AntiCheat;
#else
#if WITH_SERVER_CODE
#if WITH_CLIENT_CODE
        if (InPlatformHandle->GetEnvironment()->bIsDedicatedServer)
#endif
        {
            // This is a dedicated server and uses a different set of EAC APIs from clients.
            return MakeSharedWithEvents<FDedicatedServerAntiCheatSystemImpl>(InPlatformHandle);
        }
#endif
#if WITH_SERVER_CODE && WITH_CLIENT_CODE
        else
#endif
#if WITH_CLIENT_CODE
        {
            // This is a client either protected by EAC, or a client that should have a trusted
            // client private key for authenticating against peers.
            return MakeSharedWithEvents<FGameAntiCheatSystemImpl>(
                InPlatformHandle,
                !InPlatformHandle->GetEnvironment()->Config->GetTrustedClientPrivateKey().IsEmpty());
        }
#endif
#endif
    }
    else
    {
        // This game doesn't have Anti-Cheat enabled, so there won't be any instances of the game
        // that require EAC functionality.
        return MakeShared<FNullAntiCheatSystemImpl>();
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()