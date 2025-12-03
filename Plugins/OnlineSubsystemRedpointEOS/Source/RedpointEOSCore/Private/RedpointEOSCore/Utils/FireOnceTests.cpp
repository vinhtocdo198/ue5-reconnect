// Copyright June Rhodes. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Utils/FireOnce.h"
#include "RedpointEOSCore/Utils/FireWithin.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1259481467, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::Core::Utils;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFireOnceTests,
    "Redpoint.EOS.FireOnce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FFireOnceTests::RunTest(const FString &Parameters)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    // This test mostly exists to make sure that the FireOnce() template compiles in newer engine versions.

    {
        bool bCalled = false;
        auto Delegate =
            FireOnce(TDelegate<void(int32, const FString &)>::CreateLambda([&bCalled](int32 A, const FString &B) {
                bCalled = true;
            }));
        Delegate.ExecuteIfBound(50, TEXT("Unused"));

        TestTrue(TEXT("Underlying delegate was called"), bCalled);
    }

    {
        bool bCalled = false;
        auto Delegate = FireWithin(
            10.0f,
            TDelegate<void(int32, const FString &)>::CreateLambda([&bCalled](int32 A, const FString &B) {
                bCalled = true;
            }),
            32,
            FString(TEXT("Hello")));
        Delegate.ExecuteIfBound(50, TEXT("Unused"));

        TestTrue(TEXT("Underlying delegate was called"), bCalled);
    }

    return true;
}

}

REDPOINT_EOS_CODE_GUARD_END()