// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Utils/NamespacedEvent.h"
#include "Misc/AutomationTest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1122440761, Redpoint::EOS::Tests::Core)
{
using namespace ::Redpoint::EOS::Core::Utils;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Core_NamespacedEvent,
    "Redpoint.EOS.Core.NamespacedEvent",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FOnlineSubsystemEOS_Core_NamespacedEvent::RunTest(const FString &Parameters)
{
    TNamespacedEvent<FSimpleMulticastDelegate> Ev;

    bool bDidFireAll = false;
    bool bDidFireGame = false;
    bool bDidFireEngine = false;

    Ev[NAME_All].AddLambda([&bDidFireAll]() {
        bDidFireAll = true;
    });
    Ev[NAME_Game].AddLambda([&bDidFireGame]() {
        bDidFireGame = true;
    });
    Ev[NAME_Engine].AddLambda([&bDidFireEngine]() {
        bDidFireEngine = true;
    });

    Ev.Broadcast(NAME_Game);

    TestTrue(TEXT("Expected NAME_All handler to fire."), bDidFireAll);
    TestTrue(TEXT("Expected NAME_Game handler to fire."), bDidFireGame);
    TestFalse(TEXT("Expected NAME_Engine handler to not fire."), bDidFireEngine);

    return true;
}

}

REDPOINT_EOS_CODE_GUARD_END()