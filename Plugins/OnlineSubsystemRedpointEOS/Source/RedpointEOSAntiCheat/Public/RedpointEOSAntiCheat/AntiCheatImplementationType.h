// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(350996716, Redpoint::EOS::AntiCheat)
{

/**
 * Represents the type of Anti-Cheat implementation being used. This prevents
 * undefined behaviour when editors connect to protected servers, by stopping
 * the editor Anti-Cheat from trying to talk to an EAC protected server.
 */
enum class EAntiCheatImplementationType : uint8
{
    // This game does not use Anti-Cheat.
    Null = 0x0,

    // The Anti-Cheat protocol is using full EAC or is a trusted client.
    EasyAntiCheat = 0x1,

    // The Anti-Cheat protocol is currently FEditorAntiCheat, and we don't
    // use the real EAC interfaces.
    Editor = 0x2,
};

}

namespace Redpoint::EOS::AntiCheat
{
REDPOINT_EOS_FILE_NS_EXPORT(350996716, Redpoint::EOS::AntiCheat, EAntiCheatImplementationType)
}

REDPOINT_EOS_CODE_GUARD_END()