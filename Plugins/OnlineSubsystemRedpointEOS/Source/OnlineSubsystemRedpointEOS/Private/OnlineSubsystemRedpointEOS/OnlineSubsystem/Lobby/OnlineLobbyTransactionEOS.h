// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3771096292, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby)
{

class FOnlineLobbyTransactionEOS : public FOnlineLobbyTransaction
{
public:
    FOnlineLobbyTransactionEOS() = default;
    UE_NONCOPYABLE(FOnlineLobbyTransactionEOS);
    virtual ~FOnlineLobbyTransactionEOS() = default;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3771096292,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby,
    FOnlineLobbyTransactionEOS)
}

#endif