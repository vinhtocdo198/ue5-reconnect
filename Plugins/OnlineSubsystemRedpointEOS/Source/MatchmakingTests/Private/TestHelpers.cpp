// Copyright June Rhodes. All Rights Reserved.

#include "TestHelpers.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "OnlineSubsystemTest.h"
#include "TestBackend.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

DEFINE_LOG_CATEGORY(LogMatchmakingTests);

FName FUniqueNetIdMMTest::GetType() const
{
    return FName(TEXT("TEST"));
}
const uint8 *FUniqueNetIdMMTest::GetBytes() const
{
    return reinterpret_cast<const uint8 *>(&this->UserId);
}
int32 FUniqueNetIdMMTest::GetSize() const
{
    return sizeof(this->UserId);
}
bool FUniqueNetIdMMTest::IsValid() const
{
    return true;
}
FString FUniqueNetIdMMTest::ToString() const
{
    return FString::Printf(TEXT("%d"), this->UserId);
}
FString FUniqueNetIdMMTest::ToDebugString() const
{
    return this->ToString();
}

TSharedPtr<const FUniqueNetId> FMatchmakingAutomationTestBase::GetUserId(int InstanceIdx)
{
    return MakeShared<const FUniqueNetIdMMTest>(InstanceIdx);
}

class FWaitForMatchmakingToBeComplete : public IAutomationLatentCommand
{
private:
    class FMatchmakingAutomationTestBase *TestBase;
    int32 NextInstanceToQueue;
    TArray<FInstance> SetInstances;
    bool bIsStarted;
    int32 ElapsedTicks;

public:
    FWaitForMatchmakingToBeComplete() = delete;
    FWaitForMatchmakingToBeComplete(const FWaitForMatchmakingToBeComplete &) = delete;
    FWaitForMatchmakingToBeComplete(class FMatchmakingAutomationTestBase *InTestBase);
    virtual ~FWaitForMatchmakingToBeComplete() override = default;
    virtual bool Update() override;
};

FWaitForMatchmakingToBeComplete::FWaitForMatchmakingToBeComplete(FMatchmakingAutomationTestBase *InTestBase)
    : TestBase(InTestBase)
    , NextInstanceToQueue(0)
    , SetInstances()
    , bIsStarted(false)
    , ElapsedTicks()
{
}

bool FWaitForMatchmakingToBeComplete::Update()
{
    if (!this->bIsStarted)
    {
        this->TestBase->Engine = IMatchmakingEngine::ForciblyCreateInstanceForUnitTests();

        TSharedPtr<FTestBackend> Backend = MakeShared<FTestBackend>();
        TArray<FInstance> Instances;

        TArray<FString> PartySizes;
        this->TestBase->PartyArrangement.ParseIntoArray(PartySizes, TEXT(":"));
        int UserIdx = 0;
        for (const auto &PartySizeStr : PartySizes)
        {
            int32 PartySize = FCString::Atoi(*PartySizeStr);
            TSharedPtr<const FOnlinePartyId> PartyId;

            for (int i = 0; i < PartySize; i++)
            {
                auto Subsystem = MakeShared<FOnlineSubsystemTest, ESPMode::ThreadSafe>(Backend);
                auto UserId = this->TestBase->GetUserId(UserIdx++);

                if (i == 0)
                {
                    // We make the party, we're the host.
                    PartyId = Subsystem->CreatePartyAtStartup(*UserId);
                }
                else
                {
                    // Otherwise we join the party.
                    Subsystem->JoinPartyAtStartup(*UserId, *PartyId);
                }

                FInstance Inst;
                Inst.UserId = UserId;
                Inst.PartyId = PartyId;
                Inst.OnlineSubsystem = Subsystem;
                Inst.bIsPartyLeader = i == 0;
                Instances.Add(Inst);
            }
        }

        if (!this->TestBase->IsDelayed())
        {
            for (int i = 0; i < Instances.Num(); i++)
            {
                this->TestBase->PendingRequests.Add(
                    i,
                    this->TestBase->QueueRequestForInstance(
                        i,
                        Instances[i].PartyId,
                        Instances[i].bIsPartyLeader,
                        Instances,
                        FMatchmakingRequestComplete::CreateLambda([this, i]() {
                            this->TestBase->PendingRequests.Remove(i);
                        })));
            }
        }

        this->SetInstances = Instances;
        this->bIsStarted = true;
    }

    if (this->TestBase->IsDelayed())
    {
        this->ElapsedTicks++;

        if (this->NextInstanceToQueue < this->SetInstances.Num() &&
            this->ElapsedTicks >= this->NextInstanceToQueue * 10)
        {
            this->TestBase->PendingRequests.Add(
                this->NextInstanceToQueue,
                this->TestBase->QueueRequestForInstance(
                    this->NextInstanceToQueue,
                    this->SetInstances[this->NextInstanceToQueue].PartyId,
                    this->SetInstances[this->NextInstanceToQueue].bIsPartyLeader,
                    this->SetInstances,
                    FMatchmakingRequestComplete::CreateLambda([this, i = this->NextInstanceToQueue]() {
                        this->TestBase->PendingRequests.Remove(i);
                    })));
            this->NextInstanceToQueue++;
        }

        if (this->TestBase->PendingRequests.Num() == 0 && this->NextInstanceToQueue == this->SetInstances.Num())
        {
            return true;
        }
    }
    else
    {
        if (this->TestBase->PendingRequests.Num() == 0)
        {
            return true;
        }
    }

    return false;
}

class FCleanupAfterMatchmakingTest : public IAutomationLatentCommand
{
private:
    class FMatchmakingAutomationTestBase *TestBase;

public:
    FCleanupAfterMatchmakingTest() = delete;
    FCleanupAfterMatchmakingTest(const FCleanupAfterMatchmakingTest &) = delete;
    FCleanupAfterMatchmakingTest(class FMatchmakingAutomationTestBase *InTestBase);
    virtual ~FCleanupAfterMatchmakingTest() override = default;
    virtual bool Update() override;
};

FCleanupAfterMatchmakingTest::FCleanupAfterMatchmakingTest(FMatchmakingAutomationTestBase *InTestBase)
    : TestBase(InTestBase)
{
}

bool FCleanupAfterMatchmakingTest::Update()
{
    check(this->TestBase->Engine.IsUnique());
    this->TestBase->Engine.Reset();
    return true;
}

bool FMatchmakingAutomationTestBase::RunTestBase(const FString &Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND_PATCHED(FWaitForMatchmakingToBeComplete(this));
    ADD_LATENT_AUTOMATION_COMMAND_PATCHED(FCleanupAfterMatchmakingTest(this));
    return true;
}

void FMatchmakingAutomationTestBase::AssertMMSlotUser(
    int CurrentInstanceIdx,
    const TArray<FInstance> &Instances,
    FMatchmakingEngineResponse Response,
    int Team,
    int Slot,
    int TargetInstanceUserIdx)
{
    if (this->TestTrue(
            FString::Printf(
                TEXT("Instance(%d) matchmaking user in team %d slot %d should be user"),
                CurrentInstanceIdx,
                Team,
                Slot),
            Response.Teams[Team].Slots[Slot].Type == EMatchmakingEngineResponseTeamSlotType::User))
    {
        this->TestEqual(
            FString::Printf(
                TEXT("Instance(%d) matchmaking user in team %d slot %d should be '%s' (user %d), but was '%s'."),
                CurrentInstanceIdx,
                Team,
                Slot,
                *Instances[TargetInstanceUserIdx].UserId->ToString(),
                TargetInstanceUserIdx,
                *Response.Teams[Team].Slots[Slot].UserId->ToString()),
            *Response.Teams[Team].Slots[Slot].UserId,
            *Instances[TargetInstanceUserIdx].UserId);
    }
}

void FMatchmakingAutomationTestBase::AssertMMSlotEmpty(
    int CurrentInstanceIdx,
    const TArray<FInstance> &Instances,
    FMatchmakingEngineResponse Response,
    int Team,
    int Slot)
{
    this->TestTrue(
        FString::Printf(
            TEXT("Instance(%d) matchmaking user in team %d slot %d should be empty"),
            CurrentInstanceIdx,
            Team,
            Slot),
        Response.Teams[Team].Slots[Slot].Type == EMatchmakingEngineResponseTeamSlotType::Empty);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()