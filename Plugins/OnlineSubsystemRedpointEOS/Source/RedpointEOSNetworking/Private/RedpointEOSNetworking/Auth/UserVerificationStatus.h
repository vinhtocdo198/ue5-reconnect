// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2378164825, Redpoint::EOS::Networking::Auth)
{

enum class EUserVerificationStatus : uint8
{
    /** We haven't started verifying this user yet. */
    NotStarted,
    /** We check if the connecting user exists using the IOnlineUser interface. */
    CheckingAccountExistsFromListenServer,
    /** We check if the connecting user exists by impersonating them with EOS_Connect_Login. */
    CheckingAccountExistsFromDedicatedServer,
    /** We're checking that the user isn't sanctioned with a BAN action. */
    CheckingSanctions,
    /** We're establishing whether the client can run as an unprotected Anti-Cheat client. */
    EstablishingAntiCheatProof,
    /** We're waiting for Anti-Cheat integrity to be confirmed before finishing the connection. */
    WaitingForAntiCheatIntegrity,
    /** We've verified the user and login can proceed. */
    Verified,
    /** We couldn't verify the user and they're not permitted to connect. */
    Failed,
};

}

namespace Redpoint::EOS::Networking::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(2378164825, Redpoint::EOS::Networking::Auth, EUserVerificationStatus)
}

REDPOINT_EOS_CODE_GUARD_END()