// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineStatsInterface.h"
#include "Misc/AutomationTest.h"
#include "OnlineError.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineStatsInterface_CanWriteStats,
    "Redpoint.EOS.OnlineStatsInterface.CanWriteStats",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineStatsInterface_CanWriteStats::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](
            const IOnlineSubsystemPtr &Subsystem,
            const TSharedPtr<const FUniqueNetIdEOS> &UserId,
            const FOnDone &OnDone) {
            if (!(Subsystem.IsValid() && UserId.IsValid()))
            {
                this->AddError(FString::Printf(TEXT("Unable to init subsystem / authenticate")));
                OnDone();
                return;
            }

            auto Stats = Subsystem->GetStatsInterface();
            TestTrue("Online subsystem provides IOnlineStats interface", Stats.IsValid());

            if (!Stats.IsValid())
            {
                OnDone();
                return;
            }

            FOnlineStatsUserUpdatedStats StatToSend = FOnlineStatsUserUpdatedStats(UserId.ToSharedRef());
            StatToSend.Stats.Add(
                TEXT("TestLatest"),
                FOnlineStatUpdate(20, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));
            StatToSend.Stats.Add(
                TEXT("TestScore"),
                FOnlineStatUpdate(10, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));

            TArray<FOnlineStatsUserUpdatedStats> StatsToSend;
            StatsToSend.Add(StatToSend);

            Stats->UpdateStats(
                UserId.ToSharedRef(),
                StatsToSend,
                FOnlineStatsUpdateStatsComplete::CreateLambda([this, OnDone](const FOnlineError &ResultState) {
                    TestTrue("Can write stats update", ResultState.bSucceeded);
                    OnDone();
                }));
        });
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineStatsInterface_CanWriteStatsFloatTruncated,
    "Redpoint.EOS.OnlineStatsInterface.CanWriteStatsFloatTruncated",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineStatsInterface_CanWriteStatsFloatTruncated::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](
            const IOnlineSubsystemPtr &Subsystem,
            const TSharedPtr<const FUniqueNetIdEOS> &UserId,
            const FOnDone &OnDone) {
            if (!(Subsystem.IsValid() && UserId.IsValid()))
            {
                this->AddError(FString::Printf(TEXT("Unable to init subsystem / authenticate")));
                OnDone();
                return;
            }

            auto Stats = Subsystem->GetStatsInterface();
            TestTrue("Online subsystem provides IOnlineStats interface", Stats.IsValid());

            if (!Stats.IsValid())
            {
                OnDone();
                return;
            }

            FOnlineStatsUserUpdatedStats StatToSend = FOnlineStatsUserUpdatedStats(UserId.ToSharedRef());
            StatToSend.Stats.Add(
                TEXT("TestFloatTrunc"),
                FOnlineStatUpdate(20.0f, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));

            TArray<FOnlineStatsUserUpdatedStats> StatsToSend;
            StatsToSend.Add(StatToSend);

            Stats->UpdateStats(
                UserId.ToSharedRef(),
                StatsToSend,
                FOnlineStatsUpdateStatsComplete::CreateLambda([this, OnDone](const FOnlineError &ResultState) {
                    TestTrue("Can write stats update", ResultState.bSucceeded);
                    OnDone();
                }));
        });
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineStatsInterface_CanWriteStatsFloatEncoded,
    "Redpoint.EOS.OnlineStatsInterface.CanWriteStatsFloatEncoded",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineStatsInterface_CanWriteStatsFloatEncoded::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](
            const IOnlineSubsystemPtr &Subsystem,
            const TSharedPtr<const FUniqueNetIdEOS> &UserId,
            const FOnDone &OnDone) {
            if (!(Subsystem.IsValid() && UserId.IsValid()))
            {
                this->AddError(FString::Printf(TEXT("Unable to init subsystem / authenticate")));
                OnDone();
                return;
            }

            auto Stats = Subsystem->GetStatsInterface();
            TestTrue("Online subsystem provides IOnlineStats interface", Stats.IsValid());

            if (!Stats.IsValid())
            {
                OnDone();
                return;
            }

            FOnlineStatsUserUpdatedStats StatToSend = FOnlineStatsUserUpdatedStats(UserId.ToSharedRef());
            StatToSend.Stats.Add(
                TEXT("TestFloatEnc"),
                FOnlineStatUpdate(20.0f, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));

            TArray<FOnlineStatsUserUpdatedStats> StatsToSend;
            StatsToSend.Add(StatToSend);

            Stats->UpdateStats(
                UserId.ToSharedRef(),
                StatsToSend,
                FOnlineStatsUpdateStatsComplete::CreateLambda([this, OnDone](const FOnlineError &ResultState) {
                    TestTrue("Can write stats update", ResultState.bSucceeded);
                    OnDone();
                }));
        });
}

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineStatsInterface_CanWriteStatsDoubleEncoded,
    "Redpoint.EOS.OnlineStatsInterface.CanWriteStatsDoubleEncoded",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineStatsInterface_CanWriteStatsDoubleEncoded::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](
            const IOnlineSubsystemPtr &Subsystem,
            const TSharedPtr<const FUniqueNetIdEOS> &UserId,
            const FOnDone &OnDone) {
            if (!(Subsystem.IsValid() && UserId.IsValid()))
            {
                this->AddError(FString::Printf(TEXT("Unable to init subsystem / authenticate")));
                OnDone();
                return;
            }

            auto Stats = Subsystem->GetStatsInterface();
            TestTrue("Online subsystem provides IOnlineStats interface", Stats.IsValid());

            if (!Stats.IsValid())
            {
                OnDone();
                return;
            }

            FOnlineStatsUserUpdatedStats StatToSend = FOnlineStatsUserUpdatedStats(UserId.ToSharedRef());
            StatToSend.Stats.Add(
                TEXT("TestDoubleEnc"),
                FOnlineStatUpdate(20.0, FOnlineStatUpdate::EOnlineStatModificationType::Unknown));

            TArray<FOnlineStatsUserUpdatedStats> StatsToSend;
            StatsToSend.Add(StatToSend);

            Stats->UpdateStats(
                UserId.ToSharedRef(),
                StatsToSend,
                FOnlineStatsUpdateStatsComplete::CreateLambda([this, OnDone](const FOnlineError &ResultState) {
                    TestTrue("Can write stats update", ResultState.bSucceeded);
                    OnDone();
                }));
        });
}