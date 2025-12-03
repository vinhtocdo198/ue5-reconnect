// Copyright June Rhodes. All Rights Reserved.

#include "TestAsyncUObject.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

::Redpoint::EOS::Async::TTask<void> UTestAsyncUObject::VoidTask()
{
    co_return;
}

::Redpoint::EOS::Async::TTask<int> UTestAsyncUObject::IntTask()
{
    co_return 5;
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()