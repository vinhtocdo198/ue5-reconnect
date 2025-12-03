// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

namespace Redpoint::EOS::Core::Id
{

template <typename ValueType> using TAccountIdMap = TMap<UE::Online::FAccountId, ValueType>;

}