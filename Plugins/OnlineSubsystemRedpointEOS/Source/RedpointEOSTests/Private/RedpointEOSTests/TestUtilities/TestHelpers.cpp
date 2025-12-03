// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/TestHelpers.h"

#include "Misc/CommandLine.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(189412454, Redpoint::EOS::Tests::TestUtilities)
{

int16 TestHelpers::WorkerNum = -1;
int TestHelpers::Port(int BasePort)
{
    if (WorkerNum == -1)
    {
        if (!FParse::Value(FCommandLine::Get(), TEXT("-AutomationWorkerNum="), WorkerNum))
        {
            WorkerNum = 0;
        }
    }

    return BasePort + (WorkerNum * 25);
}

}

REDPOINT_EOS_CODE_GUARD_END()