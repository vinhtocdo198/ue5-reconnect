// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Utils/IndexedTable.h"

#include "Misc/AutomationTest.h"
#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3264241354, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::Core::Utils;

class FUniqueNetIdTestImpl : public FUniqueNetId
{
public:
    FString UniqueNetIdStr;

    static FUniqueNetIdRef Create(const FString &InUniqueNetId)
    {
        return MakeShareable(new FUniqueNetIdTestImpl(InUniqueNetId));
    }
    virtual ~FUniqueNetIdTestImpl() override = default;

    virtual FName GetType() const override
    {
        return FName(TEXT("Test"));
    }
    virtual const uint8 *GetBytes() const override
    {
        return reinterpret_cast<const uint8 *>(UniqueNetIdStr.GetCharArray().GetData());
    }
    virtual int32 GetSize() const override
    {
        return static_cast<int32>(UniqueNetIdStr.GetCharArray().GetTypeSize()) * UniqueNetIdStr.GetCharArray().Num();
    }
    virtual bool IsValid() const override
    {
        return !UniqueNetIdStr.IsEmpty();
    }
    virtual FString ToString() const override
    {
        return UniqueNetIdStr;
    }
    virtual FString ToDebugString() const override
    {
        return UniqueNetIdStr;
    }
    virtual uint32 GetTypeHash() const override
    {
        return GetTypeHashHelper(UniqueNetIdStr);
    }

private:
    explicit FUniqueNetIdTestImpl(const FString &InUniqueNetId)
        : UniqueNetIdStr(InUniqueNetId)
    {
    }
};

typedef TTuple<FString, FString> FStringString;
static const auto CIndexedTableTestStringString = FIndexedColumnDefinition<FStringString>(
    FName(TEXT("StringString")),
    [](const FStringString &A, const FStringString &B) {
        return A.Key == B.Key && A.Value == B.Value;
    },
    [](const FStringString &K) {
        return HashCombine(GetTypeHash(K.Key), GetTypeHash(K.Value));
    });
static const auto CIndexedTableTestInt = FIndexedColumnDefinition<int>(
    FName(TEXT("int")),
    [](const int &A, const int &B) {
        return A == B;
    },
    [](const int &K) {
        return GetTypeHash(K);
    });
static const auto CIndexedTableTestString = FIndexedColumnDefinition<FString>(
    FName(TEXT("FString")),
    [](const FString &A, const FString &B) {
        return A == B;
    },
    [](const FString &K) {
        return GetTypeHash(K);
    });
static const auto CIndexedTableTestUniqueNetId = FIndexedColumnDefinition<FUniqueNetIdRef>(
    FName(TEXT("FUniqueNetIdRef")),
    [](const FUniqueNetIdRef &A, const FUniqueNetIdRef &B) {
        return *A == *B;
    },
    [](const FUniqueNetIdRef &K) {
        return GetTypeHash(*K);
    });

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FIndexedTableTests,
    "Redpoint.EOS.IndexedTable",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FIndexedTableTests::RunTest(const FString &Parameters)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    FIndexedTable Table;
    Table.AddIndexedColumn(CIndexedTableTestStringString, true)
        .AddIndexedColumn(CIndexedTableTestInt, true)
        .AddIndexedColumn(CIndexedTableTestString, false)
        .AddIndexedColumn(CIndexedTableTestUniqueNetId, true);

    Table.AddRow(
        {CIndexedTableTestStringString.Create(TTuple<FString, FString>("A", "B")),
         CIndexedTableTestInt.Create(1),
         CIndexedTableTestString.Create("A"),
         CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("1234"))});
    Table.AddRow(
        {CIndexedTableTestStringString.Create(TTuple<FString, FString>("C", "D")),
         CIndexedTableTestInt.Create(2),
         CIndexedTableTestString.Create("A"),
         CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("5678"))});

    TestTrue(
        TEXT("Expected to find row by 'A/B'"),
        Table.FindFirstByValue(CIndexedTableTestStringString.Create(TTuple<FString, FString>("A", "B"))).IsValid());
    TestTrue(
        TEXT("Expected to find row by 'C/D'"),
        Table.FindFirstByValue(CIndexedTableTestStringString.Create(TTuple<FString, FString>("C", "D"))).IsValid());
    TestFalse(
        TEXT("Expected to not find row by 'E/F'"),
        Table.FindFirstByValue(CIndexedTableTestStringString.Create(TTuple<FString, FString>("E", "F"))).IsValid());
    TestTrue(TEXT("Expected to find row by '1'"), Table.FindFirstByValue(CIndexedTableTestInt.Create(1)).IsValid());
    TestTrue(TEXT("Expected to find row by '2'"), Table.FindFirstByValue(CIndexedTableTestInt.Create(2)).IsValid());
    TestFalse(
        TEXT("Expected to not find row by '3'"),
        Table.FindFirstByValue(CIndexedTableTestInt.Create(3)).IsValid());
    TestTrue(
        TEXT("Expected to find row by 'uid:1234'"),
        Table.FindFirstByValue(CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("1234"))).IsValid());
    TestTrue(
        TEXT("Expected to find row by 'uid:5678'"),
        Table.FindFirstByValue(CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("5678"))).IsValid());
    TestFalse(
        TEXT("Expected to not find row by 'uid:9090'"),
        Table.FindFirstByValue(CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("9090"))).IsValid());

    TestEqual(TEXT("Expected to find two rows"), Table.FindByValue(CIndexedTableTestString.Create("A")).Num(), 2);

    auto Row1 = Table.FindFirstByValue(CIndexedTableTestInt.Create(1));
    auto Row2 = Table.FindFirstByValue(CIndexedTableTestInt.Create(2));

    TestEqual(
        TEXT("Expected row 1 string/string value to match 'A/B'"),
        CIndexedTableTestStringString.Get(Row1.ToSharedRef()),
        TTuple<FString, FString>("A", "B"));
    TestEqual(TEXT("Expected row 1 int value to match 1"), CIndexedTableTestInt.Get(Row1.ToSharedRef()), 1);
    TestEqual(
        TEXT("Expected row 1 string value to match 'A'"),
        CIndexedTableTestString.Get(Row1.ToSharedRef()),
        TEXT("A"));
    TestEqual(
        TEXT("Expected row 1 uid value to match '1234'"),
        CIndexedTableTestUniqueNetId.Get(Row1.ToSharedRef())->ToString(),
        TEXT("1234"));

    TestEqual(
        TEXT("Expected row 2 string/string value to match 'C/D'"),
        CIndexedTableTestStringString.Get(Row2.ToSharedRef()),
        TTuple<FString, FString>("C", "D"));
    TestEqual(TEXT("Expected row 2 int value to match 1"), CIndexedTableTestInt.Get(Row2.ToSharedRef()), 2);
    TestEqual(
        TEXT("Expected row 2 string value to match 'A'"),
        CIndexedTableTestString.Get(Row2.ToSharedRef()),
        TEXT("A"));
    TestEqual(
        TEXT("Expected row 2 uid value to match '5678'"),
        CIndexedTableTestUniqueNetId.Get(Row2.ToSharedRef())->ToString(),
        TEXT("5678"));

    auto RowsByUid1234 = Table.FindByValue(CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("1234")));
    TestEqual(TEXT("Expected to find 1 row by UID 1234"), RowsByUid1234.Num(), 1);

    Table.RemoveRow(Row2);

    TestTrue(
        TEXT("Expected to find row by 'A/B'"),
        Table.FindFirstByValue(CIndexedTableTestStringString.Create(TTuple<FString, FString>("A", "B"))).IsValid());
    TestFalse(
        TEXT("Expected to not find row by 'C/D'"),
        Table.FindFirstByValue(CIndexedTableTestStringString.Create(TTuple<FString, FString>("C", "D"))).IsValid());
    TestFalse(
        TEXT("Expected to not find row by 'E/F'"),
        Table.FindFirstByValue(CIndexedTableTestStringString.Create(TTuple<FString, FString>("E", "F"))).IsValid());
    TestTrue(TEXT("Expected to find row by '1'"), Table.FindFirstByValue(CIndexedTableTestInt.Create(1)).IsValid());
    TestFalse(
        TEXT("Expected to not find row by '2'"),
        Table.FindFirstByValue(CIndexedTableTestInt.Create(2)).IsValid());
    TestFalse(
        TEXT("Expected to not find row by '3'"),
        Table.FindFirstByValue(CIndexedTableTestInt.Create(3)).IsValid());
    TestTrue(
        TEXT("Expected to find row by 'uid:1234'"),
        Table.FindFirstByValue(CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("1234"))).IsValid());
    TestFalse(
        TEXT("Expected to not find row by 'uid:5678'"),
        Table.FindFirstByValue(CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("5678"))).IsValid());
    TestFalse(
        TEXT("Expected to not find row by 'uid:9090'"),
        Table.FindFirstByValue(CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("9090"))).IsValid());

    TestEqual(TEXT("Expected to find one row"), Table.FindByValue(CIndexedTableTestString.Create("A")).Num(), 1);

    Table.RemoveRow(Row1);

    TestFalse(
        TEXT("Expected to not find row by 'A/B'"),
        Table.FindFirstByValue(CIndexedTableTestStringString.Create(TTuple<FString, FString>("A", "B"))).IsValid());
    TestFalse(
        TEXT("Expected to not find row by 'C/D'"),
        Table.FindFirstByValue(CIndexedTableTestStringString.Create(TTuple<FString, FString>("C", "D"))).IsValid());
    TestFalse(
        TEXT("Expected to not find row by 'E/F'"),
        Table.FindFirstByValue(CIndexedTableTestStringString.Create(TTuple<FString, FString>("E", "F"))).IsValid());
    TestFalse(
        TEXT("Expected to not find row by '1'"),
        Table.FindFirstByValue(CIndexedTableTestInt.Create(1)).IsValid());
    TestFalse(
        TEXT("Expected to not find row by '2'"),
        Table.FindFirstByValue(CIndexedTableTestInt.Create(2)).IsValid());
    TestFalse(
        TEXT("Expected to not find row by '3'"),
        Table.FindFirstByValue(CIndexedTableTestInt.Create(3)).IsValid());
    TestFalse(
        TEXT("Expected to not find row by 'uid:1234'"),
        Table.FindFirstByValue(CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("1234"))).IsValid());
    TestFalse(
        TEXT("Expected to not find row by 'uid:5678'"),
        Table.FindFirstByValue(CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("5678"))).IsValid());
    TestFalse(
        TEXT("Expected to not find row by 'uid:9090'"),
        Table.FindFirstByValue(CIndexedTableTestUniqueNetId.Create(FUniqueNetIdTestImpl::Create("9090"))).IsValid());

    TestEqual(TEXT("Expected to find no rows"), Table.FindByValue(CIndexedTableTestString.Create("A")).Num(), 0);

    return true;
}

}

REDPOINT_EOS_CODE_GUARD_END()