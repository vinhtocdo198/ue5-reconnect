// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSConfig/Types/StatTypingRule.h"

namespace Redpoint::EOS::Config
{

TArray<TTuple<FString, EStatTypingRule>> ParseStatTypingRules(const TArray<FString> &Value);

}