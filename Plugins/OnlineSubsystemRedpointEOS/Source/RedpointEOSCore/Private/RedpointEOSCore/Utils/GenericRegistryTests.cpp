// Copyright June Rhodes. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/GenericRegistry.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4038350483, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::Core::Utils;

class FTestEntry
{
public:
    int ExpectedPriority;

    FTestEntry(int InExpectedPriority)
        : ExpectedPriority(InExpectedPriority)
    {
    }
    FTestEntry(const FTestEntry &) = default;
    FTestEntry(FTestEntry &&) = default;
    ~FTestEntry() = default;
};

class FTestRegistry : public TGenericRegistry<FTestEntry, FTestRegistry>
{
    REDPOINT_EOS_DECLARE_GENERIC_REGISTRY_BODY()
};

REDPOINT_EOS_DEFINE_GENERIC_REGISTRY(FTestRegistry);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGenericRegistryTest,
    "Redpoint.EOS.GenericRegistry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FGenericRegistryTest::RunTest(const FString &Parameters)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    auto Entries = TArray<TSharedRef<FTestEntry>>{
        MakeShared<FTestEntry>(2000),
        MakeShared<FTestEntry>(100),
        MakeShared<FTestEntry>(100000),
        MakeShared<FTestEntry>(0),
        MakeShared<FTestEntry>(400)};

    for (const auto &Entry : Entries)
    {
        FTestRegistry::Register(Entry, Entry->ExpectedPriority);
    }

    int LastEntry = 10000000;
    for (const auto &Entry : FTestRegistry::Get())
    {
        this->TestTrue(TEXT("Expected priority to be lower than last priority"), Entry->ExpectedPriority < LastEntry);
        LastEntry = Entry->ExpectedPriority;
    }

    for (const auto &Entry : Entries)
    {
        FTestRegistry::Unregister(Entry);
    }

    return true;
}

}

REDPOINT_EOS_CODE_GUARD_END()