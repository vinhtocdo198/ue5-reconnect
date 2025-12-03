// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_BEGIN()
#include "Misc/AutomationTest.h"
#include "RedpointEOSTests/TestUtilities/AsyncHotReloadableAutomationTestBase.h"
#include "RedpointEOSTests/TestUtilities/CleanupSubsystemsCommand.h"
#include "RedpointEOSTests/TestUtilities/Delegates.h"
#include "RedpointEOSTests/TestUtilities/LambdaCommand.h"
REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_END()

REDPOINT_EOS_CODE_GUARD_BEGIN()

// @note: Due to the way IMPLEMENT_SIMPLE_AUTOMATION_TEST_PRIVATE works, we can't
// specify a fully qualified name for the base class (because it also gets inserted
// as a constructor base call).
using ::Redpoint::EOS::Tests::TestUtilities::FAsyncHotReloadableAutomationTestBase;

#define ADD_LATENT_AUTOMATION_COMMAND_PATCHED(ClassDeclaration)                                                        \
    FAutomationTestFramework::Get().SetRequestedTestFilter(EAutomationTestFlags::ProductFilter);                       \
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new ClassDeclaration));

#define IMPLEMENT_ASYNC_AUTOMATION_TEST(TClass, PrettyName, TFlags)                                                    \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST_PRIVATE(                                                                          \
        TClass##_Base,                                                                                                 \
        FAsyncHotReloadableAutomationTestBase,                                                                         \
        PrettyName,                                                                                                    \
        TFlags,                                                                                                        \
        __FILE__,                                                                                                      \
        __LINE__)                                                                                                      \
    bool TClass##_Base::RunTest(const FString &Parameters)                                                             \
    {                                                                                                                  \
        ADD_LATENT_AUTOMATION_COMMAND_PATCHED(::Redpoint::EOS::Tests::TestUtilities::FLambdaCommand(                   \
            this,                                                                                                      \
            [this](const ::Redpoint::EOS::Tests::TestUtilities::FOnDone &OnDone) {                                     \
                this->RunAsyncTest(OnDone);                                                                            \
            }));                                                                                                       \
        ADD_LATENT_AUTOMATION_COMMAND_PATCHED(::Redpoint::EOS::Tests::TestUtilities::FCleanupSubsystemsCommand(this)); \
        return true;                                                                                                   \
    }                                                                                                                  \
    class TClass : TClass##_Base                                                                                       \
    {                                                                                                                  \
    public:                                                                                                            \
        TClass(const FString &InName)                                                                                  \
            : TClass##_Base(InName){};                                                                                 \
                                                                                                                       \
    protected:                                                                                                         \
        virtual void RunAsyncTest(const ::Redpoint::EOS::Tests::TestUtilities::FOnDone &OnDone) override;              \
    };                                                                                                                 \
    namespace                                                                                                          \
    {                                                                                                                  \
    TClass TClass##AutomationTestInstance(TEXT(#TClass));                                                              \
    }

#define IMPLEMENT_MANAGED_ASYNC_AUTOMATION_TEST(TClass, PrettyName, TFlags, TManagingClass)                            \
    class TManagingClass;                                                                                              \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST_PRIVATE(                                                                          \
        TClass##_Base,                                                                                                 \
        FAsyncHotReloadableAutomationTestBase,                                                                         \
        PrettyName,                                                                                                    \
        TFlags,                                                                                                        \
        __FILE__,                                                                                                      \
        __LINE__)                                                                                                      \
    bool TClass##_Base::RunTest(const FString &Parameters)                                                             \
    {                                                                                                                  \
        ADD_LATENT_AUTOMATION_COMMAND_PATCHED(::Redpoint::EOS::Tests::TestUtilities::FLambdaCommand(                   \
            this,                                                                                                      \
            [this](const ::Redpoint::EOS::Tests::TestUtilities::FOnDone &OnDone) {                                     \
                this->RunAsyncTest(OnDone);                                                                            \
            }));                                                                                                       \
        ADD_LATENT_AUTOMATION_COMMAND_PATCHED(::Redpoint::EOS::Tests::TestUtilities::FCleanupSubsystemsCommand(this)); \
        return true;                                                                                                   \
    }                                                                                                                  \
    class TClass : TClass##_Base                                                                                       \
    {                                                                                                                  \
        friend TManagingClass;                                                                                         \
                                                                                                                       \
    private:                                                                                                           \
        TSharedPtr<TManagingClass> ManagingInstance;                                                                   \
                                                                                                                       \
    public:                                                                                                            \
        TClass(const FString &InName)                                                                                  \
            : TClass##_Base(InName){};                                                                                 \
                                                                                                                       \
    protected:                                                                                                         \
        void CreateManagingInstance(                                                                                   \
            const ::Redpoint::EOS::Tests::TestUtilities::FOnDone &OnDone,                                              \
            const TFunction<void(const TSharedRef<TManagingClass> &)> &OnInstanceCreated);                             \
        virtual void RunAsyncTest(const ::Redpoint::EOS::Tests::TestUtilities::FOnDone &OnDone) override               \
        {                                                                                                              \
            ::Redpoint::EOS::Tests::TestUtilities::FOnDone OnCleanup = [this, OnDone]() {                              \
                this->ManagingInstance = nullptr;                                                                      \
                OnDone();                                                                                              \
            };                                                                                                         \
            this->CreateManagingInstance(OnCleanup, [this](const TSharedRef<TManagingClass> &Inst) {                   \
                this->ManagingInstance = Inst;                                                                         \
            });                                                                                                        \
        }                                                                                                              \
    };                                                                                                                 \
    namespace                                                                                                          \
    {                                                                                                                  \
    TClass TClass##AutomationTestInstance(TEXT(#TClass));                                                              \
    }

REDPOINT_EOS_CODE_GUARD_END()