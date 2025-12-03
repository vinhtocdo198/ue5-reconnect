// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Net/DataBunch.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class URedpointEOSControlChannel;

namespace REDPOINT_EOS_FILE_NS_ID(734137399, Redpoint::EOS::Networking::Auth)
{

typedef TDelegate<bool(URedpointEOSControlChannel *ControlChannel, FInBunch &Bunch)> FAuthPhaseRoute;

}

namespace Redpoint::EOS::Networking::Auth
{
REDPOINT_EOS_FILE_NS_EXPORT(734137399, Redpoint::EOS::Networking::Auth, FAuthPhaseRoute);
}

REDPOINT_EOS_CODE_GUARD_END()