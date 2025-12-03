// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAntiCheat/AntiCheatSession.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3248392819, Redpoint::EOS::AntiCheat)
{

FAntiCheatSession::FAntiCheatSession()
    : SessionInstanceGuid(FGuid::NewGuid())
{
}

const FGuid &FAntiCheatSession::GetInstanceGuid() const
{
    return this->SessionInstanceGuid;
}

FString FAntiCheatSession::ToString() const
{
    return this->SessionInstanceGuid.ToString();
}

}

REDPOINT_EOS_CODE_GUARD_END()