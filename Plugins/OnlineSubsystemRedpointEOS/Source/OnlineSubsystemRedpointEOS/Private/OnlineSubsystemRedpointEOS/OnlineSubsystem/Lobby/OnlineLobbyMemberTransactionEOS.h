// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(505106068, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby)
{

class FOnlineLobbyMemberTransactionEOS : public FOnlineLobbyMemberTransaction
{
public:
    FOnlineLobbyMemberTransactionEOS() = default;
    UE_NONCOPYABLE(FOnlineLobbyMemberTransactionEOS);
    virtual ~FOnlineLobbyMemberTransactionEOS() = default;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(
    505106068,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby,
    FOnlineLobbyMemberTransactionEOS)
}

#endif