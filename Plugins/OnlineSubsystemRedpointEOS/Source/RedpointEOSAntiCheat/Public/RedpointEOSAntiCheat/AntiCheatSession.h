// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3248392819, Redpoint::EOS::AntiCheat)
{

class REDPOINTEOSANTICHEAT_API FAntiCheatSession : public TSharedFromThis<FAntiCheatSession>
{
private:
    FGuid SessionInstanceGuid;

public:
    FAntiCheatSession();
    virtual ~FAntiCheatSession() = default;
    UE_NONCOPYABLE(FAntiCheatSession);

    const FGuid &GetInstanceGuid() const;
    FString ToString() const;
};

}

namespace Redpoint::EOS::AntiCheat
{
REDPOINT_EOS_FILE_NS_EXPORT(3248392819, Redpoint::EOS::AntiCheat, FAntiCheatSession)
}

REDPOINT_EOS_CODE_GUARD_END()