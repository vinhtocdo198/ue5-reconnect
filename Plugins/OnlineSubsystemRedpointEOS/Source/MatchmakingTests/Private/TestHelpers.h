// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Engine/Engine.h"
#include "Engine/EngineBaseTypes.h"
#include "Misc/AutomationTest.h"
#include "Online/CoreOnline.h"
#include "OnlineSubsystem.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "Templates/SharedPointer.h"
#include <functional>
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

DECLARE_LOG_CATEGORY_EXTERN(LogMatchmakingTests, All, All);

DECLARE_DELEGATE(FMatchmakingRequestComplete);

class FUniqueNetIdMMTest : public FUniqueNetId
{
private:
    int32 UserId;

public:
    FUniqueNetIdMMTest(int32 InUserId)
        : UserId(InUserId) {};

    virtual FName GetType() const override;
    virtual const uint8 *GetBytes() const override;
    virtual int32 GetSize() const override;
    virtual bool IsValid() const override;
    virtual FString ToString() const override;
    virtual FString ToDebugString() const override;
    FORCEINLINE int32 GetIntValue() const
    {
        return this->UserId;
    }
};

struct FInstance
{
    IOnlineSubsystemPtr OnlineSubsystem;
    TSharedPtr<const FUniqueNetId> UserId;
    TSharedPtr<const FOnlinePartyId> PartyId;
    bool bIsPartyLeader;
};

class FMatchmakingAutomationTestBase : public FAutomationTestBase
{
    friend class FWaitForMatchmakingToBeComplete;
    friend class FCleanupAfterMatchmakingTest;

protected:
    FString PartyArrangement;
    TMap<int, FMatchmakingEngineRequestHandle> PendingRequests;
    TSharedPtr<IMatchmakingEngine> Engine;

    bool RunTestBase(const FString &Parameters);

    FMatchmakingEngineRequestHandle GetRequestHandleForInstance(int InstanceIdx)
    {
        return this->PendingRequests[InstanceIdx];
    };

public:
    FMatchmakingAutomationTestBase(const FString &InName, const bool bInComplexTask)
        : FAutomationTestBase(InName, bInComplexTask)
    {
    }
    virtual ~FMatchmakingAutomationTestBase() override = default;

    virtual bool IsDelayed() const = 0;

    virtual FMatchmakingEngineRequestHandle QueueRequestForInstance(
        int InstanceIdx,
        TSharedPtr<const FOnlinePartyId> PartyId,
        bool bIsPartyLeader,
        TArray<FInstance> Instances,
        FMatchmakingRequestComplete OnDone) = 0;

    virtual TSharedPtr<const FUniqueNetId> GetUserId(int InstanceIdx);

protected:
    void AssertMMSlotUser(
        int CurrentInstanceIdx,
        const TArray<FInstance> &Instances,
        FMatchmakingEngineResponse Response,
        int Team,
        int Slot,
        int TargetInstanceUserIdx);
    void AssertMMSlotEmpty(
        int CurrentInstanceIdx,
        const TArray<FInstance> &Instances,
        FMatchmakingEngineResponse Response,
        int Team,
        int Slot);
};

#define ADD_LATENT_AUTOMATION_COMMAND_PATCHED(ClassDeclaration)                                                        \
    FAutomationTestFramework::Get().SetRequestedTestFilter(EAutomationTestFlags::ProductFilter);                       \
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new ClassDeclaration));

#define IMPLEMENT_MATCHMAKING_PARTY_AUTOMATION_TEST(TClass, PrettyName, TFlags, TPartyArrangement)                     \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST_PRIVATE(                                                                          \
        TClass##_Base,                                                                                                 \
        FMatchmakingAutomationTestBase,                                                                                \
        PrettyName,                                                                                                    \
        TFlags,                                                                                                        \
        __FILE__,                                                                                                      \
        __LINE__)                                                                                                      \
    bool TClass##_Base::RunTest(const FString &Parameters)                                                             \
    {                                                                                                                  \
        return this->RunTestBase(Parameters);                                                                          \
    }                                                                                                                  \
    class TClass : TClass##_Base                                                                                       \
    {                                                                                                                  \
    public:                                                                                                            \
        TClass(const FString &InName)                                                                                  \
            : TClass##_Base(InName)                                                                                    \
        {                                                                                                              \
            this->PartyArrangement = TPartyArrangement;                                                                \
        };                                                                                                             \
        virtual ~TClass() override = default;                                                                          \
                                                                                                                       \
        virtual bool IsDelayed() const override                                                                        \
        {                                                                                                              \
            return false;                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        virtual FMatchmakingEngineRequestHandle QueueRequestForInstance(                                               \
            int InstanceIdx,                                                                                           \
            TSharedPtr<const FOnlinePartyId> PartyId,                                                                  \
            bool bIsPartyLeader,                                                                                       \
            TArray<FInstance> Instances,                                                                               \
            FMatchmakingRequestComplete OnDone) override;                                                              \
    };                                                                                                                 \
    namespace                                                                                                          \
    {                                                                                                                  \
    TClass TClass##AutomationTestInstance(TEXT(#TClass));                                                              \
    }

#define IMPLEMENT_MATCHMAKING_AUTOMATION_TEST(TClass, PrettyName, TFlags, TInstanceCount)                              \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST_PRIVATE(                                                                          \
        TClass##_Base,                                                                                                 \
        FMatchmakingAutomationTestBase,                                                                                \
        PrettyName,                                                                                                    \
        TFlags,                                                                                                        \
        __FILE__,                                                                                                      \
        __LINE__)                                                                                                      \
    bool TClass##_Base::RunTest(const FString &Parameters)                                                             \
    {                                                                                                                  \
        return this->RunTestBase(Parameters);                                                                          \
    }                                                                                                                  \
    class TClass : TClass##_Base                                                                                       \
    {                                                                                                                  \
    public:                                                                                                            \
        TClass(const FString &InName)                                                                                  \
            : TClass##_Base(InName)                                                                                    \
        {                                                                                                              \
            FString Arrangement = TEXT("");                                                                            \
            for (int i = 0; i < TInstanceCount; i++)                                                                   \
            {                                                                                                          \
                if (Arrangement.IsEmpty())                                                                             \
                {                                                                                                      \
                    Arrangement += TEXT("1");                                                                          \
                }                                                                                                      \
                else                                                                                                   \
                {                                                                                                      \
                    Arrangement += TEXT(":1");                                                                         \
                }                                                                                                      \
            }                                                                                                          \
            this->PartyArrangement = Arrangement;                                                                      \
        };                                                                                                             \
        virtual ~TClass() override = default;                                                                          \
                                                                                                                       \
        virtual bool IsDelayed() const override                                                                        \
        {                                                                                                              \
            return false;                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        virtual FMatchmakingEngineRequestHandle QueueRequestForInstance(                                               \
            int InstanceIdx,                                                                                           \
            TSharedPtr<const FOnlinePartyId> PartyId,                                                                  \
            bool bIsPartyLeader,                                                                                       \
            TArray<FInstance> Instances,                                                                               \
            FMatchmakingRequestComplete OnDone) override                                                               \
        {                                                                                                              \
            return this->QueueRequestForInstance(InstanceIdx, Instances, OnDone);                                      \
        }                                                                                                              \
                                                                                                                       \
        FMatchmakingEngineRequestHandle QueueRequestForInstance(                                                       \
            int InstanceIdx,                                                                                           \
            TArray<FInstance> Instances,                                                                               \
            FMatchmakingRequestComplete OnDone);                                                                       \
    };                                                                                                                 \
    namespace                                                                                                          \
    {                                                                                                                  \
    TClass TClass##AutomationTestInstance(TEXT(#TClass));                                                              \
    }

#define IMPLEMENT_MATCHMAKING_AUTOMATION_TEST_DELAYED(TClass, PrettyName, TFlags, TInstanceCount)                      \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST_PRIVATE(                                                                          \
        TClass##_Base,                                                                                                 \
        FMatchmakingAutomationTestBase,                                                                                \
        PrettyName,                                                                                                    \
        TFlags,                                                                                                        \
        __FILE__,                                                                                                      \
        __LINE__)                                                                                                      \
    bool TClass##_Base::RunTest(const FString &Parameters)                                                             \
    {                                                                                                                  \
        return this->RunTestBase(Parameters);                                                                          \
    }                                                                                                                  \
    class TClass : TClass##_Base                                                                                       \
    {                                                                                                                  \
    public:                                                                                                            \
        TClass(const FString &InName)                                                                                  \
            : TClass##_Base(InName)                                                                                    \
        {                                                                                                              \
            FString Arrangement = TEXT("");                                                                            \
            for (int i = 0; i < TInstanceCount; i++)                                                                   \
            {                                                                                                          \
                if (Arrangement.IsEmpty())                                                                             \
                {                                                                                                      \
                    Arrangement += TEXT("1");                                                                          \
                }                                                                                                      \
                else                                                                                                   \
                {                                                                                                      \
                    Arrangement += TEXT(":1");                                                                         \
                }                                                                                                      \
            }                                                                                                          \
            this->PartyArrangement = Arrangement;                                                                      \
        };                                                                                                             \
        virtual ~TClass() override = default;                                                                          \
                                                                                                                       \
        virtual bool IsDelayed() const override                                                                        \
        {                                                                                                              \
            return true;                                                                                               \
        }                                                                                                              \
                                                                                                                       \
        virtual FMatchmakingEngineRequestHandle QueueRequestForInstance(                                               \
            int InstanceIdx,                                                                                           \
            TSharedPtr<const FOnlinePartyId> PartyId,                                                                  \
            bool bIsPartyLeader,                                                                                       \
            TArray<FInstance> Instances,                                                                               \
            FMatchmakingRequestComplete OnDone) override                                                               \
        {                                                                                                              \
            return this->QueueRequestForInstance(InstanceIdx, Instances, OnDone);                                      \
        }                                                                                                              \
                                                                                                                       \
        FMatchmakingEngineRequestHandle QueueRequestForInstance(                                                       \
            int InstanceIdx,                                                                                           \
            TArray<FInstance> Instances,                                                                               \
            FMatchmakingRequestComplete OnDone);                                                                       \
    };                                                                                                                 \
    namespace                                                                                                          \
    {                                                                                                                  \
    TClass TClass##AutomationTestInstance(TEXT(#TClass));                                                              \
    }

#define IMPLEMENT_ADVANCED_MATCHMAKING_AUTOMATION_TEST(TClass, PrettyName, TFlags, TInstanceCount)                     \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST_PRIVATE(                                                                          \
        TClass##_Base,                                                                                                 \
        FMatchmakingAutomationTestBase,                                                                                \
        PrettyName,                                                                                                    \
        TFlags,                                                                                                        \
        __FILE__,                                                                                                      \
        __LINE__)                                                                                                      \
    bool TClass##_Base::RunTest(const FString &Parameters)                                                             \
    {                                                                                                                  \
        return this->RunTestBase(Parameters);                                                                          \
    }                                                                                                                  \
    class TClass : TClass##_Base                                                                                       \
    {                                                                                                                  \
    public:                                                                                                            \
        TClass(const FString &InName)                                                                                  \
            : TClass##_Base(InName)                                                                                    \
        {                                                                                                              \
            FString Arrangement = TEXT("");                                                                            \
            for (int i = 0; i < TInstanceCount; i++)                                                                   \
            {                                                                                                          \
                if (Arrangement.IsEmpty())                                                                             \
                {                                                                                                      \
                    Arrangement += TEXT("1");                                                                          \
                }                                                                                                      \
                else                                                                                                   \
                {                                                                                                      \
                    Arrangement += TEXT(":1");                                                                         \
                }                                                                                                      \
            }                                                                                                          \
            this->PartyArrangement = Arrangement;                                                                      \
        };                                                                                                             \
        virtual ~TClass() override = default;                                                                          \
                                                                                                                       \
        virtual bool IsDelayed() const override                                                                        \
        {                                                                                                              \
            return false;                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        virtual FMatchmakingEngineRequestHandle QueueRequestForInstance(                                               \
            int InstanceIdx,                                                                                           \
            TSharedPtr<const FOnlinePartyId> PartyId,                                                                  \
            bool bIsPartyLeader,                                                                                       \
            TArray<FInstance> Instances,                                                                               \
            FMatchmakingRequestComplete OnDone) override                                                               \
        {                                                                                                              \
            return this->QueueRequestForInstance(InstanceIdx, Instances, OnDone);                                      \
        }                                                                                                              \
                                                                                                                       \
        FMatchmakingEngineRequestHandle QueueRequestForInstance(                                                       \
            int InstanceIdx,                                                                                           \
            TArray<FInstance> Instances,                                                                               \
            FMatchmakingRequestComplete OnDone);                                                                       \
                                                                                                                       \
        virtual TSharedPtr<const FUniqueNetId> GetUserId(int InstanceIdx) override;                                    \
    };                                                                                                                 \
    namespace                                                                                                          \
    {                                                                                                                  \
    TClass TClass##AutomationTestInstance(TEXT(#TClass));                                                              \
    }

#define ASSERT_MM_SLOT_USER(Team, Slot, UserIndex)                                                                     \
    this->AssertMMSlotUser(InstanceIdx, Instances, Response, Team, Slot, UserIndex);
#define ASSERT_MM_SLOT_EMPTY(Team, Slot) this->AssertMMSlotEmpty(InstanceIdx, Instances, Response, Team, Slot);

#endif

REDPOINT_EOS_CODE_GUARD_END()