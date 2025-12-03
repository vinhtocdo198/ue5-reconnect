// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAntiCheat/AntiCheatSession.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3635414841, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;

class FNullAntiCheatSession : public FAntiCheatSession
{
public:
    FNullAntiCheatSession() = default;
    UE_NONCOPYABLE(FNullAntiCheatSession);
    virtual ~FNullAntiCheatSession() override = default;
};

}

namespace Redpoint::EOS::AntiCheat
{
REDPOINT_EOS_FILE_NS_EXPORT(3635414841, Redpoint::EOS::AntiCheat, FNullAntiCheatSession)
}

REDPOINT_EOS_CODE_GUARD_END()