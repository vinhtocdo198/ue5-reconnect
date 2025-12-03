// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAntiCheat/AntiCheatSession.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/AccountIdMap.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3125823358, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Id;

class FEditorAntiCheatSession : public FAntiCheatSession
{
public:
    struct FRegisteredPlayer
    {
        EOS_EAntiCheatCommonClientType ClientType;
        EOS_EAntiCheatCommonClientPlatform ClientPlatform;
    };

    bool bIsServer;
    UE::Online::FAccountId HostUserId;
    bool bIsDedicatedServerSession;
    TAccountIdMap<TSharedPtr<FRegisteredPlayer>> RegisteredPlayers;

    FEditorAntiCheatSession(UE::Online::FAccountId InHostUserId);
    UE_NONCOPYABLE(FEditorAntiCheatSession);
    virtual ~FEditorAntiCheatSession() override = default;
};

}

namespace Redpoint::EOS::AntiCheat
{
REDPOINT_EOS_FILE_NS_EXPORT(3125823358, Redpoint::EOS::AntiCheat, FEditorAntiCheatSession)
}

REDPOINT_EOS_CODE_GUARD_END()