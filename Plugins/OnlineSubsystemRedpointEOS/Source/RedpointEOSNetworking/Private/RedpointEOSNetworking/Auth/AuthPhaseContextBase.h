// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Delegates/DelegateCombinations.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "OnlineSubsystemRedpointEOS/Private/EOSControlChannel.h"
#include "OnlineSubsystemRedpointEOS/Private/NetworkingAuth/AuthPhaseFailureCode.h"
#include "OnlineSubsystemRedpointEOS/Private/NetworkingAuth/NetworkHelpers.h"
#include "OnlineSubsystemRedpointEOS/Shared/AntiCheat/AntiCheat.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSNetDriver.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(795199830, Redpoint::EOS::Networking::Auth)
{

DECLARE_MULTICAST_DELEGATE_TwoParams(FAuthPhasesCompleted, EAuthPhaseFailureCode, const FString &ErrorMessage);

class IAuthPhaseContext
{
public:
    [[nodiscard]] virtual bool GetControlChannel(UEOSControlChannel *&OutControlChannel) const = 0;
    [[nodiscard]] virtual bool GetConnection(UNetConnection *&OutConnection) const = 0;
    [[nodiscard]] virtual bool GetOSS(FOnlineSubsystemEOS *&OutOSS) = 0;
    [[nodiscard]] virtual bool GetConfig(const Redpoint::EOS::Config::IConfig *&OutConfig) = 0;
    [[nodiscard]] virtual bool GetRole(EEOSNetDriverRole &OutRole) = 0;
    [[nodiscard]] virtual bool GetSocketSubsystem(ISocketSubsystem *&OutSocketSubsystem) = 0;
    virtual void Finish(EAuthPhaseFailureCode ErrorCode) = 0;
};

}

REDPOINT_EOS_CODE_GUARD_END()