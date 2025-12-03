// Copyright June Rhodes. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/AliasRoomFeature.h"
#include "RedpointEOSRooms/RoomIdAndAliasMap.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(753448449, Redpoint::EOS::Rooms::Tests)
{
using namespace ::Redpoint::EOS::Rooms;

class FTestAliasRoomFeature : public Features::IAliasRoomFeature
{
private:
    FName Name;

public:
    FTestAliasRoomFeature(const FName &InName)
        : Name(InName) {};
    UE_NONCOPYABLE(FTestAliasRoomFeature);
    virtual ~FTestAliasRoomFeature() override = default;

    virtual const FName &Alias() const override
    {
        return this->Name;
    }

    virtual TSharedPtr<Features::IRoomFeatureRequestBase> GetFeatureRequestForUpdate() const override
    {
        return nullptr;
    }
};

class FTestRoom : public IRoom
{
private:
    FRoomIdRef RoomId;
    FRoomOwnerRef RoomOwner;
    TMap<FString, FRoomAttribute> RoomAttributes;
    TSharedPtr<Features::IRoomFeature> AliasFeature;
    TArray<FRoomMemberRef> RoomMembers;
    FOnRoomMemberJoined OnRoomMemberJoinedValue;
    FOnRoomMemberLeft OnRoomMemberLeftValue;
    FOnRoomMemberUpdated OnRoomMemberUpdatedValue;
    FOnRoomMemberPromoted OnRoomMemberPromotedValue;
    FRoomUserId BoundForLocalUser;

public:
    FTestRoom(const FRoomIdRef &InRoomId, const FRoomOwnerRef &InRoomOwner)
        : RoomId(InRoomId)
        , RoomOwner(InRoomOwner)
        , RoomAttributes()
        , AliasFeature()
        , RoomMembers()
        , OnRoomMemberJoinedValue()
        , OnRoomMemberLeftValue()
        , OnRoomMemberUpdatedValue()
        , OnRoomMemberPromotedValue()
        , BoundForLocalUser() {};

    FTestRoom(const FRoomIdRef &InRoomId, const FRoomOwnerRef &InRoomOwner, const FName &InAlias)
        : RoomId(InRoomId)
        , RoomOwner(InRoomOwner)
        , RoomAttributes()
        , AliasFeature(MakeShared<FTestAliasRoomFeature>(InAlias))
        , RoomMembers()
        , OnRoomMemberJoinedValue()
        , OnRoomMemberLeftValue()
        , OnRoomMemberUpdatedValue()
        , OnRoomMemberPromotedValue()
        , BoundForLocalUser() {};

    virtual const FRoomIdRef &GetRoomId() const override
    {
        return this->RoomId;
    }

    virtual FRoomOwnerRef GetRoomOwner() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return this->RoomOwner;
    }

    virtual FName GetRoomNamespace() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return FName(TEXT("AutomationTests"));
    }

    virtual uint32 GetRoomMaxMembers() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return 1;
    }

    virtual const TMap<FString, FRoomAttribute> &GetRoomAttributes() const override
    {
        return this->RoomAttributes;
    }

    virtual TSet<FName> GetRoomFeatureNames() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        TSet<FName> Result;
        if (this->AliasFeature.IsValid())
        {
            Result.Add(FTestAliasRoomFeature::GetFeatureName());
        }
        return Result;
    }

    virtual TSharedPtr<Features::IRoomFeature> GetRoomFeatureBase(const FName &FeatureName) const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return this->AliasFeature;
    }

    virtual const TArray<FRoomMemberRef> &GetMembers() const override
    {
        return this->RoomMembers;
    }

    virtual FOnRoomMemberJoined &OnRoomMemberJoined() override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return this->OnRoomMemberJoinedValue;
    }

    virtual FOnRoomMemberLeft &OnRoomMemberLeft() override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return this->OnRoomMemberLeftValue;
    }

    virtual FOnRoomMemberUpdated &OnRoomMemberUpdated() override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return this->OnRoomMemberUpdatedValue;
    }

    virtual FOnRoomMemberPromoted &OnRoomMemberPromoted() override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return this->OnRoomMemberPromotedValue;
    }

    virtual FRoomUserId GetBoundForLocalUser() const override
    {
        REDPOINT_EOS_USE_LLM_TAG(Rooms);

        return this->BoundForLocalUser;
    }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRoomIdAndAliasMap_ById,
    "Redpoint.EOS.RoomIdAndAliasMap.ById",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter)

bool FRoomIdAndAliasMap_ById::RunTest(const FString &Parameters)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    FRoomOwnerRef RoomOwner = MakeShared<FRoomOwner>("TEST");
    FRoomIdRef RoomAId = MakeShared<FRoomId>("Test", TEXT("A"));
    FRoomIdRef RoomBId = MakeShared<FRoomId>("Test", TEXT("B"));
    FRoomRef RoomA = MakeShared<FTestRoom>(RoomAId, RoomOwner);
    FRoomRef RoomB = MakeShared<FTestRoom>(RoomBId, RoomOwner);

    FRoomIdAndAliasMap Map;
    TestFalse("Map does not contain A", Map.Contains(RoomAId));
    TestFalse("Map does not contain B", Map.Contains(RoomBId));
    Map.Add(RoomA);
    TestTrue("Map contains A", Map.Contains(RoomAId));
    TestFalse("Map does not contain B", Map.Contains(RoomBId));
    TestEqual("Map lookup A == Room A", RoomA, Map[RoomAId]);
    Map.Add(RoomB);
    TestTrue("Map contains A", Map.Contains(RoomAId));
    TestTrue("Map contains B", Map.Contains(RoomBId));
    TestEqual("Map lookup A == Room A", RoomA, Map[RoomAId]);
    TestEqual("Map lookup B == Room B", RoomB, Map[RoomBId]);
    Map.Remove(RoomAId);
    TestFalse("Map does not contain A", Map.Contains(RoomAId));
    TestTrue("Map contains B", Map.Contains(RoomBId));
    TestEqual("Map lookup B == Room B", RoomB, Map[RoomBId]);
    Map.Remove(RoomBId);
    TestFalse("Map does not contain A", Map.Contains(RoomAId));
    TestFalse("Map does not contain B", Map.Contains(RoomBId));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRoomIdAndAliasMap_ByAlias,
    "Redpoint.EOS.RoomIdAndAliasMap.ByAlias",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter)

bool FRoomIdAndAliasMap_ByAlias::RunTest(const FString &Parameters)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    FRoomOwnerRef RoomOwner = MakeShared<FRoomOwner>("TEST");
    FRoomRef RoomA = MakeShared<FTestRoom>(MakeShared<FRoomId>("Test", TEXT("A")), RoomOwner, "A");
    FRoomRef RoomB = MakeShared<FTestRoom>(MakeShared<FRoomId>("Test", TEXT("B")), RoomOwner, "B");

    FRoomIdAndAliasMap Map;
    TestFalse("Map does not contain A", Map.Contains("A"));
    TestFalse("Map does not contain B", Map.Contains("B"));
    Map.Add(RoomA);
    TestTrue("Map contains A", Map.Contains("A"));
    TestFalse("Map does not contain B", Map.Contains("B"));
    TestEqual("Map lookup A == Room A", RoomA, Map["A"]);
    Map.Add(RoomB);
    TestTrue("Map contains A", Map.Contains("A"));
    TestTrue("Map contains B", Map.Contains("B"));
    TestEqual("Map lookup A == Room A", RoomA, Map["A"]);
    TestEqual("Map lookup B == Room B", RoomB, Map["B"]);
    Map.Remove("A");
    TestFalse("Map does not contain A", Map.Contains("A"));
    TestTrue("Map contains B", Map.Contains("B"));
    TestEqual("Map lookup B == Room B", RoomB, Map["B"]);
    Map.Remove("B");
    TestFalse("Map does not contain A", Map.Contains("A"));
    TestFalse("Map does not contain B", Map.Contains("B"));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRoomIdAndAliasMap_ByIteration,
    "Redpoint.EOS.RoomIdAndAliasMap.Iteration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter)

bool FRoomIdAndAliasMap_ByIteration::RunTest(const FString &Parameters)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms);

    FRoomOwnerRef RoomOwner = MakeShared<FRoomOwner>("TEST");
    FRoomIdRef RoomAId = MakeShared<FRoomId>("Test", TEXT("A"));
    FRoomIdRef RoomBId = MakeShared<FRoomId>("Test", TEXT("B"));
    FRoomRef RoomA = MakeShared<FTestRoom>(RoomAId, RoomOwner);
    FRoomRef RoomB = MakeShared<FTestRoom>(RoomBId, RoomOwner);

    FRoomIdAndAliasMap Map;
    Map.Add(RoomA);
    Map.Add(RoomB);

    bool bFoundRoomA = false;
    bool bFoundRoomB = false;
    for (const auto &Room : Map)
    {
        TestTrue("Expected room to be one we added", Room == RoomA || Room == RoomB);
        bFoundRoomA = bFoundRoomA || Room == RoomA;
        bFoundRoomB = bFoundRoomB || Room == RoomB;
    }
    TestTrue("Iterated over A", bFoundRoomA);
    TestTrue("Iterated over B", bFoundRoomB);

    return true;
}

}

REDPOINT_EOS_CODE_GUARD_END()