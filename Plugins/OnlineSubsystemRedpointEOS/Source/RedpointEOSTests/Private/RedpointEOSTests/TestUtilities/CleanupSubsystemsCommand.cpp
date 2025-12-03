// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TestUtilities/CleanupSubsystemsCommand.h"

#include "OnlineSubsystemRedpointEOSModule.h"
#include "RedpointEOSTests/TestUtilities/AsyncHotReloadableAutomationTestBase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2476792889, Redpoint::EOS::Tests::TestUtilities)
{

FCleanupSubsystemsCommand::FCleanupSubsystemsCommand(FAsyncHotReloadableAutomationTestBase *InTestBase)
    : TestBase(InTestBase)
{
}

bool FCleanupSubsystemsCommand::Update()
{
    TArray<FName> InstanceNames;
    for (const auto &Subsystem : this->TestBase->RegisteredSubsystems)
    {
        InstanceNames.Add(Subsystem->GetInstanceName());
        Subsystem->Shutdown();
    }
    this->TestBase->RegisteredSubsystems.Empty();

    FOnlineSubsystemRedpointEOSModule &OSSModule =
        FModuleManager::GetModuleChecked<FOnlineSubsystemRedpointEOSModule>(FName(TEXT("OnlineSubsystemRedpointEOS")));
    for (auto InstanceName : InstanceNames)
    {
        check(!OSSModule.HasInstance(InstanceName));
    }

    return true;
}

}

REDPOINT_EOS_CODE_GUARD_END()