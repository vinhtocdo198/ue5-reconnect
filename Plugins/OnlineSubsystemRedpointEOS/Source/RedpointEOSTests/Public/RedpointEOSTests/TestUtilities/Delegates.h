// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3120076726, Redpoint::EOS::Tests::TestUtilities)
{

typedef TFunction<void(TFunction<void()> OnDone)> FLambdaTest;
typedef TFunction<void()> FOnDone;

}

namespace Redpoint::EOS::Tests::TestUtilities
{
REDPOINT_EOS_FILE_NS_EXPORT(3120076726, Redpoint::EOS::Tests::TestUtilities, FLambdaTest)
REDPOINT_EOS_FILE_NS_EXPORT(3120076726, Redpoint::EOS::Tests::TestUtilities, FOnDone)
}

REDPOINT_EOS_CODE_GUARD_END()