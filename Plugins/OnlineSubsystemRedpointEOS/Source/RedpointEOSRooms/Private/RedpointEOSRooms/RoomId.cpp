// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/RoomId.h"

#include "Misc/AutomationTest.h"
#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(518246272, Redpoint::EOS::Rooms)
{
using namespace ::Redpoint::EOS::Rooms;

FRoomId::FRoomId(const FName &InProviderName, const FString &InRoomId)
    : ProviderName(InProviderName)
    , RoomId(InRoomId)
{
}

const FName &FRoomId::GetProviderName() const
{
    return this->ProviderName;
}

const FString &FRoomId::GetRoomId() const
{
    return this->RoomId;
}

FString FRoomId::ToString() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return FString::Printf(TEXT("%s:%s"), *this->ProviderName.ToString(), *this->RoomId);
}

TSharedPtr<const FRoomId> FRoomId::FromString(const FString &InRoomId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    TArray<FString> Components;
    InRoomId.ParseIntoArray(Components, TEXT(":"));
    if (Components.Num() != 2)
    {
        return nullptr;
    }
    return MakeShared<FRoomId>(FName(*Components[0]), Components[1]);
}

bool FRoomId::IsEqual(const FRoomId &OtherRoomId) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->ProviderName.IsEqual(OtherRoomId.ProviderName) && this->RoomId == OtherRoomId.RoomId;
}

bool FRoomId::IsEqual(const TSharedRef<const FRoomId> &OtherRoomId) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    return this->IsEqual(*OtherRoomId);
}

void FRoomId::SplitToInt32(int32 &Out1, int32 &Out2, int32 &Out3, int32 &Out4, int32 &Out5) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    Out1 = this->ProviderName.IsEqual(FName(TEXT("Session"))) ? 0 : 1;

    uint8 AsBinary[16];

    check((this->RoomId.Len() / 2) == 16);
    HexToBytes(this->RoomId, AsBinary);

    Out2 = reinterpret_cast<int32 *>(AsBinary)[0];
    Out3 = reinterpret_cast<int32 *>(AsBinary)[1];
    Out4 = reinterpret_cast<int32 *>(AsBinary)[2];
    Out5 = reinterpret_cast<int32 *>(AsBinary)[3];
}

TSharedPtr<const FRoomId> FRoomId::JoinFromInt32(int32 In1, int32 In2, int32 In3, int32 In4, int32 In5)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    uint8 AsBinary[16];

    reinterpret_cast<int32 *>(AsBinary)[0] = In2;
    reinterpret_cast<int32 *>(AsBinary)[1] = In3;
    reinterpret_cast<int32 *>(AsBinary)[2] = In4;
    reinterpret_cast<int32 *>(AsBinary)[3] = In5;

    return MakeShared<FRoomId>(In1 == 0 ? FName(TEXT("Session")) : FName(TEXT("Lobby")), BytesToHexLower(AsBinary, 16));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRoomIdInt32Roundtrip,
    "Redpoint.EOS.RoomSystem.Int32Roundtrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FRoomIdInt32Roundtrip::RunTest(const FString &Parameters)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    for (int k = 0; k < 5000; k++)
    {
        FString RoomIdStr;
        for (int i = 0; i < 16; i++)
        {
            RoomIdStr += FString::Printf(TEXT("%02x"), FMath::Rand() % 256);
        }

        auto RoomId = MakeShared<FRoomId>(FMath::RandBool() ? FName(TEXT("Session")) : FName(TEXT("Lobby")), RoomIdStr);

        int32 Val1, Val2, Val3, Val4, Val5;
        RoomId->SplitToInt32(Val1, Val2, Val3, Val4, Val5);
        auto Decoded = FRoomId::JoinFromInt32(Val1, Val2, Val3, Val4, Val5);

        TestTrue(
            FString::Printf(
                TEXT("%d: Expected '%s' to round trip, but was decoded as '%s'."),
                k,
                *RoomId->ToString(),
                *Decoded->ToString()),
            RoomId->IsEqual(Decoded.ToSharedRef()));
    }

    return true;
}

}

REDPOINT_EOS_CODE_GUARD_END()