// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/ConfigurationEnumLookup.h"

#include "Misc/AutomationTest.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Class.h"

namespace REDPOINT_EOS_FILE_NS_ID(839941062, Redpoint::EOS::Editor::Config)
{
using namespace ::Redpoint::EOS::Editor::Config;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FEnumLookupTest,
    "Redpoint.EOS.EnumLookup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FEnumLookupTest::RunTest(const FString &Parameters)
{
    const UEnum *Result = LookupEnum("EEOSApiVersion");
    TestNotNull("Lookup of EEOSApiVersion enum should not fail", Result);
    return true;
}

}