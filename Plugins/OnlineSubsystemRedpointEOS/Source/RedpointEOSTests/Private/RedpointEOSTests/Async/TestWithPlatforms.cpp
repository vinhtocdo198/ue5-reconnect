// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Async/TestWithPlatforms.h"

#include "RedpointEOSAsync/AsyncAutomationTest.h"
#include "RedpointEOSCore/InstancePool.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSTests/TestUtilities/EOSConfigEASLogin.h"
#include "RedpointEOSTests/TestUtilities/TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4127373091, Redpoint::EOS::Tests::Async)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Identity::Requests;
using namespace ::Redpoint::EOS::Tests::TestUtilities;

static TTask<FError> Login(
    TSharedRef<FAutomationTestBase> Assert,
    TSharedRef<IIdentitySystem> IdentitySystem,
    FIdentityUserSlot Slot)
{
    auto Result = TTask<FError>::Deferred();
    IdentitySystem->Login(
        FLoginRequest(
            Slot,
            TEXT("AUTOMATED_TESTING"),
            FString::Printf(TEXT("CreateOnDemand:%s"), *Assert->GetTestName()),
            FString::Printf(TEXT("%d"), TestHelpers::Port(Slot))),
        IIdentitySystem::FOnLoginComplete::CreateLambda([Result](FError ErrorCode, FIdentityUserPtr NewUser) {
            Result.SetValue(ErrorCode);
        }));
    return Result;
}

static TTask<FError> Logout(
    TSharedRef<FAutomationTestBase> Assert,
    TSharedRef<IIdentitySystem> IdentitySystem,
    FIdentityUserSlot Slot)
{
    auto Result = TTask<FError>::Deferred();
    IdentitySystem->Logout(
        FLogoutRequest(Slot),
        IIdentitySystem::FOnLogoutComplete::CreateLambda([Result](FError ErrorCode) {
            Result.SetValue(ErrorCode);
        }));
    return Result;
}

static TTask<void>::With<ETaskBinding::Static> TestWithPlatformsInternal(
    TSharedRef<FAutomationTestBase> Assert,
    TArray<int32> InstanceLayout,
    TFunction<TTask<void>(TSharedRef<FAutomationTestBase> Assert, TArray<FPlatformHandle> Platforms)> Test)
{
    TArray<FPlatformOptionalRefCountedHandle> PlatformRefs;
    TArray<FPlatformHandle> Platforms;

    for (int i = 0; i < InstanceLayout.Num(); i++)
    {
        FPlatformOptionalRefCountedHandle PlatformRef = IInstancePool::Pool().CreateWithConfig(
            FName(*FString::Printf(TEXT("%s_Test%02d"), *Assert->GetTestName(), i)),
            MakeShared<FEOSConfigEASLogin>(),
            false);
        auto Platform = PlatformRef->Instance();

        auto IdentitySystem = Platform->GetSystem<IIdentitySystem>();

        for (int u = 0; u < InstanceLayout[i]; u++)
        {
            if (!Assert->TestTrue(
                    TEXT("Expected automation test login to succeed."),
                    (co_await Login(Assert, IdentitySystem, u)).WasSuccessful()))
            {
                co_return;
            }
        }

        PlatformRefs.Add(PlatformRef);
        Platforms.Add(Platform);
    }

    co_await Test(Assert, Platforms);

    for (int i = 0; i < InstanceLayout.Num(); i++)
    {
        auto IdentitySystem = Platforms[i]->GetSystem<IIdentitySystem>();

        for (int u = 0; u < InstanceLayout[i]; u++)
        {
            if (!Assert->TestTrue(
                    TEXT("Expected automation test logout to succeed."),
                    (co_await Logout(Assert, IdentitySystem, u)).WasSuccessful()))
            {
                co_return;
            }
        }
    }

    Platforms.Empty();
    PlatformRefs.Empty();
    co_return;
}

TFunction<TTask<void>(TSharedRef<FAutomationTestBase> Assert)> TestWithPlatforms(
    TArray<int32> InstanceLayout,
    TFunction<TTask<void>(TSharedRef<FAutomationTestBase> Assert, TArray<FPlatformHandle> Platforms)> Test)
{
    return [InstanceLayout, Test](TSharedRef<FAutomationTestBase> Assert) -> TTask<void> {
        return TestWithPlatformsInternal(Assert, InstanceLayout, Test);
    };
}

}

REDPOINT_EOS_CODE_GUARD_END()
