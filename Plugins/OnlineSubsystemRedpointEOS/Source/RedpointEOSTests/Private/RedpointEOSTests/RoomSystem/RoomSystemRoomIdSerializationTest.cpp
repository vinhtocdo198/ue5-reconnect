// Copyright June Rhodes. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/RoomId.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(541626002, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Rooms;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOnlineSubsystemEOS_RoomSystem_RoomIdSerialization,
    "Redpoint.EOS.RoomSystem.RoomIdSerialization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FOnlineSubsystemEOS_RoomSystem_RoomIdSerialization::RunTest(const FString &Parameters)
{
    auto RoomId = MakeShared<FRoomId>(FName(TEXT("Test")), TEXT("RoomId"));

    auto RoomIdSerialized = RoomId->ToString();
    TestEqual(
        TEXT("Expected serialized room ID to be represented as 'Test:RoomId'."),
        RoomIdSerialized,
        TEXT("Test:RoomId"));

    auto RoomIdDeserialized = FRoomId::FromString(RoomIdSerialized);
    if (TestTrue(TEXT("Expected serialized room ID to be deserializable."), RoomIdDeserialized.IsValid()))
    {
        TestEqual(TEXT("Expected provider to be 'Test'"), RoomIdDeserialized->GetProviderName(), FName(TEXT("Test")));
        TestEqual(TEXT("Expected room ID to be 'RoomId'"), RoomIdDeserialized->GetRoomId(), FString(TEXT("RoomId")));
    }

    return true;
}

}

REDPOINT_EOS_CODE_GUARD_END()