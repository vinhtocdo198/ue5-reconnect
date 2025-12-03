// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetDriver.h"
#include "Misc/AutomationTest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3947667502, Redpoint::EOS::Networking)
{

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FNetDriverTest,
    "Redpoint.EOS.Networking.NetDriverConfig",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FNetDriverTest::RunTest(const FString &Parameters)
{
    // Only run this test on CI/CD, where we know we've set up the project correctly.
    bool ShouldTestConfig = !FPlatformMisc::GetEnvironmentVariable(TEXT("CI_JOB_JWT_V1")).IsEmpty();
    if (ShouldTestConfig)
    {
        URedpointEOSNetDriver *CDO = URedpointEOSNetDriver::StaticClass()->GetDefaultObject<URedpointEOSNetDriver>();
        if (this->TestEqual("Net driver has overridden MaxNetTickRate", CDO->MaxNetTickRate, 240))
        {
            this->AddInfo(
                TEXT("Net driver successfully had settings overridden by DefaultOnlineSubsystemRedpointEOS.ini"));
        }
        else
        {
            this->AddWarning(
                TEXT("Net driver DID NOT have settings overridden by DefaultOnlineSubsystemRedpointEOS.ini"));
        }
    }
    else
    {
        this->AddWarning(TEXT("Net driver test IS NOT RUNNING because this test is not running on CI/CD"));
    }
    return true;
}

}

REDPOINT_EOS_CODE_GUARD_END()