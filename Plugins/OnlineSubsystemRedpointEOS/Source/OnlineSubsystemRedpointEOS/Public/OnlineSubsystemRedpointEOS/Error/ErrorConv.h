// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineError.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::Error
{

FOnlineError ONLINESUBSYSTEMREDPOINTEOS_API AsOSSError(const Redpoint::EOS::API::FError &InError);

}