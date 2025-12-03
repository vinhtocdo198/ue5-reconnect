// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAntiCheat/EditorAntiCheatSession.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3125823358, Redpoint::EOS::AntiCheat)
{

FEditorAntiCheatSession::FEditorAntiCheatSession(UE::Online::FAccountId InHostUserId)
    : bIsServer(false)
    , HostUserId(InHostUserId)
    , bIsDedicatedServerSession(false)
    , RegisteredPlayers()
{
}

}

REDPOINT_EOS_CODE_GUARD_END()