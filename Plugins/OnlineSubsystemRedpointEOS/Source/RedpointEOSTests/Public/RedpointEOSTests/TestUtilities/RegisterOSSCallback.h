// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/TestUtilities/AsyncHotReloadableAutomationTestBase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3219441436, Redpoint::EOS::Tests::TestUtilities)
{
using namespace Redpoint::EOS::Tests::TestUtilities;

template <typename TService, typename TDelegate, typename... TArgs>
TFunction<void()> RegisterOSSCallback(
    FAsyncHotReloadableAutomationTestBase *TestBase,
    TSharedPtr<TService, ESPMode::ThreadSafe> ServicePtr,
    int32 LocalUserNum,
    FDelegateHandle (TService::*AddFn)(int32 LocalUserNum, const TDelegate &Delegate),
    void (TService::*ClearFn)(int32 LocalUserNum, FDelegateHandle &Handle),
    TFunction<void(int32 LocalUserNum, TArgs... Args)> Callback)
{
    TService *Service = ServicePtr.Get();
    TSharedPtr<FDelegateHandle> DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle = (Service->*AddFn)(
        LocalUserNum,
        TDelegate::CreateLambda(
            [TestBase, LocalUserNum, Callback, DelegateHandle](int32 LocalUserNumCb, TArgs... Args) {
                if (LocalUserNum != LocalUserNumCb)
                {
                    return;
                }
                if (TestBase->FireOnceFlags.Contains(*DelegateHandle))
                {
                    if (TestBase->FireOnceFlags[*DelegateHandle])
                    {
                        return;
                    }
                    TestBase->FireOnceFlags.Add(*DelegateHandle, true);
                }
                Callback(LocalUserNumCb, Args...);
                TestBase->FireOnceFlags.Remove(*DelegateHandle);
                TestBase->RegisteredDelegates[*DelegateHandle]();
            }));
    FDelegateHandle DelegateHandleNonPtr = *DelegateHandle;
    TestBase->FireOnceFlags.Add(DelegateHandleNonPtr, false);
    TestBase->RegisteredDelegates.Add(
        DelegateHandleNonPtr,
        [TestBase, LocalUserNum, DelegateHandleNonPtr, ClearFn, ServicePtr]() {
            FDelegateHandle DelegateHandleNonPtrCopy = DelegateHandleNonPtr;
            TService *ServiceLcl = ServicePtr.Get();
            (ServiceLcl->*ClearFn)(LocalUserNum, DelegateHandleNonPtrCopy);
            TestBase->RegisteredDelegates.Remove(DelegateHandleNonPtr);
        });
    return [TestBase, DelegateHandleNonPtr]() {
        TestBase->RegisteredDelegates[DelegateHandleNonPtr]();
    };
}

template <typename TService, typename TDelegate, typename... TArgs>
TFunction<void()> RegisterOSSCallback(
    FAsyncHotReloadableAutomationTestBase *TestBase,
    TSharedPtr<TService, ESPMode::ThreadSafe> ServicePtr,
    FDelegateHandle (TService::*AddFn)(const TDelegate &Delegate),
    void (TService::*ClearFn)(FDelegateHandle &Handle),
    TFunction<void(TArgs... Args)> Callback)
{
    TService *Service = ServicePtr.Get();
    TSharedPtr<FDelegateHandle> DelegateHandle = MakeShared<FDelegateHandle>();
    *DelegateHandle =
        (Service->*AddFn)(TDelegate::CreateLambda([TestBase, ServicePtr, Callback, DelegateHandle](TArgs... Args) {
            if (TestBase->FireOnceFlags.Contains(*DelegateHandle))
            {
                if (TestBase->FireOnceFlags[*DelegateHandle])
                {
                    return;
                }
                TestBase->FireOnceFlags.Add(*DelegateHandle, true);
            }
            Callback(Args...);
            TestBase->FireOnceFlags.Remove(*DelegateHandle);
            TestBase->RegisteredDelegates[*DelegateHandle]();
        }));
    FDelegateHandle DelegateHandleNonPtr = *DelegateHandle;
    TestBase->RegisteredDelegates.Add(DelegateHandleNonPtr, [TestBase, DelegateHandleNonPtr, ClearFn, ServicePtr]() {
        FDelegateHandle DelegateHandleNonPtrCopy = DelegateHandleNonPtr;
        TService *ServiceLcl = ServicePtr.Get();
        (ServiceLcl->*ClearFn)(DelegateHandleNonPtrCopy);
        TestBase->RegisteredDelegates.Remove(DelegateHandleNonPtr);
    });
    return [TestBase, DelegateHandleNonPtr]() {
        TestBase->RegisteredDelegates[DelegateHandleNonPtr]();
    };
}

}

namespace Redpoint::EOS::Tests::TestUtilities
{
REDPOINT_EOS_FILE_NS_EXPORT(3219441436, Redpoint::EOS::Tests::TestUtilities, RegisterOSSCallback)
}

REDPOINT_EOS_CODE_GUARD_END()