// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystem.h"
#include "RedpointEOSAPI/FlagEnum.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSTests/TestUtilities/AsyncHotReloadableAutomationTestBase.h"
#include "RedpointEOSTests/TestUtilities/Delegates.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(366403318, Redpoint::EOS::Tests::TestUtilities)
{
using namespace ::Redpoint::EOS::Tests::TestUtilities;

class REDPOINTEOSTESTS_API FInstanceFactory
{
public:
    enum class EFlag : uint8
    {
        None,

        DedicatedServer = 0x2,

        EpicGames = 0x4,
    };

    struct FInstanceUserDescriptor
    {
    public:
        int32 GlobalUserNum;
        int32 LocalUserNum;
        FString CredentialId;
        FString CredentialToken;
    };

    struct FInstanceDescriptor
    {
    public:
        int32 InstanceId;
        FName SubsystemName;
        TArray<FInstanceUserDescriptor> Users;
    };

    struct FInstance
    {
    public:
        TWeakPtr<IOnlineSubsystem, ESPMode::ThreadSafe> Subsystem;
        TSharedPtr<const FUniqueNetIdEOS> UserId;
        FName WorldContextHandle;
    };

    struct FEpicGamesAutomatedTestingCredential
    {
    public:
        FString Username;
        FString Password;
    };

    static bool LoadEpicGamesAutomatedTestingCredentials(
        TArray<FEpicGamesAutomatedTestingCredential> &OutEpicGamesAutomatedTestingCredentials);

    typedef TFunction<void(const TArray<FInstance> &Instances, const FOnDone &OnDone)> FOnInstancesCreated;
    typedef TFunction<void(
        const IOnlineSubsystemPtr &Subsystem,
        const TSharedPtr<const FUniqueNetIdEOS> &UserId,
        const FOnDone &OnDone)>
        FOnSingleInstanceCreated;

    static void CreateInstances(
        FAsyncHotReloadableAutomationTestBase *Test,
        const TArray<int32> &InstanceLayout,
        const FOnDone &OnDoneFinal,
        EFlag Flags,
        const FOnInstancesCreated &Callback);
    static void CreateInstances(
        FAsyncHotReloadableAutomationTestBase *Test,
        const TArray<int32> &InstanceLayout,
        const FOnDone &OnDoneFinal,
        EFlag Flags,
        const FOnSingleInstanceCreated &Callback);
};

REDPOINT_EOS_DECLARE_FLAG_ENUMERATION_OPERATORS(FInstanceFactory::EFlag)

}

namespace Redpoint::EOS::Tests::TestUtilities
{
REDPOINT_EOS_FILE_NS_EXPORT(366403318, Redpoint::EOS::Tests::TestUtilities, FInstanceFactory)
}

REDPOINT_EOS_CODE_GUARD_END()