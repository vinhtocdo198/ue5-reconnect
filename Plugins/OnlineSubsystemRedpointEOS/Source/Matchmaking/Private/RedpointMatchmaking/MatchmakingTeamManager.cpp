// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingTeamManager.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

FMatchmakingEngineTaskTeamManager::FMatchmakingEngineTaskTeamManager(const FMatchmakingEngineRequest &InRequest)
    : HostUserId()
    , bHasPendingBatch()
    , TotalCapacity()
    , Teams()
{
    // todo: This will need to be updated for local clients, which won't have a host configuration (and probably
    // shouldn't have a team manager available either, since their state should entirely come from the lobby)...
    check(InRequest.HostConfiguration.IsValid());

    // Set up the basic state.
    this->SetupInitialState(InRequest);
}

void FMatchmakingEngineTaskTeamManager::SetupInitialState(const FMatchmakingEngineRequest &InRequest)
{
    this->HostUserId = InRequest.UserId;
    this->TotalCapacity = 0;
    FMatchmakingTeamId TeamIndex = 0;
    for (const auto &TeamCapacity : InRequest.HostConfiguration->TeamCapacities)
    {
        TMap<FMatchmakingSlotId, TSharedRef<FMatchmakingSlotInfo>> Slots;
        for (FMatchmakingSlotId i = 0; i < TeamCapacity; i++)
        {
            Slots.Emplace(i, MakeShared<FMatchmakingSlotInfo>());
            this->TotalCapacity++;
        }
        this->Teams.Add(TeamIndex, Slots);
        TeamIndex++;
    }
    this->bHasPendingBatch = false;

    // Assign the request's user to the first team, first slot. This user is the host of this lobby, so we
    // set them up in the initial state.
    if (InRequest.UserId.IsValid() && InRequest.HostConfiguration->TeamCapacities.Num() > 0 &&
        InRequest.HostConfiguration->TeamCapacities[0] > 0)
    {
        this->Teams[0][0]->AssignedUserId = InRequest.UserId.ToSharedRef();
        this->Teams[0][0]->bIsInPendingBatch = false;
        this->Teams[0][0]->bConnected = true;
        this->Teams[0][0]->OpenskillRating = FOpenskillRating();
    }

    this->DumpTeamManagerState();
}

void FMatchmakingEngineTaskTeamManager::DumpTeamManagerState() const
{
#if !defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING
    if (!this->HostUserId.IsValid())
    {
        return;
    }

    UE_LOG(
        LogMatchmakingTeamManagement,
        Verbose,
        TEXT("%s: Team Manager Pending: %s"),
        *this->HostUserId->ToString(),
        this->bHasPendingBatch ? TEXT("Yes") : TEXT("No"));
    UE_LOG(
        LogMatchmakingTeamManagement,
        Verbose,
        TEXT("%s: Team Manager Total Capacity: %d"),
        *this->HostUserId->ToString(),
        this->TotalCapacity);
    UE_LOG(
        LogMatchmakingTeamManagement,
        Verbose,
        TEXT("%s:  Team | Slot | Pending? | Connected? | Skill Mu | Skill Sigma | Skill Ord | Assigned User Id"),
        *this->HostUserId->ToString());
    UE_LOG(
        LogMatchmakingTeamManagement,
        Verbose,
        TEXT("%s: ------|------|----------|------------|----------|-------------|-----------|------------------"),
        *this->HostUserId->ToString());
    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            FString UserId = TEXT("");
            if (SlotKV.Value->AssignedUserId.IsValid())
            {
                UserId = SlotKV.Value->AssignedUserId->ToString();
            }
            UE_LOG(
                LogMatchmakingTeamManagement,
                Verbose,
                TEXT("%s:  %s | %s | %s | %s | %s | %s | %s | %s"),
                *this->HostUserId->ToString(),
                *FString::Printf(TEXT("%d"), TeamKV.Key).LeftPad(4),
                *FString::Printf(TEXT("%d"), SlotKV.Key).LeftPad(4),
                *FString(SlotKV.Value->bIsInPendingBatch ? TEXT("Yes") : TEXT("")).LeftPad(8),
                *FString(SlotKV.Value->bConnected ? TEXT("Yes") : TEXT("")).LeftPad(10),
                *(!SlotKV.Value->AssignedUserId.IsValid()
                      ? TEXT("")
                      : FString::Printf(TEXT("%.2f"), SlotKV.Value->OpenskillRating.GetMu()))
                     .LeftPad(8),
                *(!SlotKV.Value->AssignedUserId.IsValid()
                      ? TEXT("")
                      : FString::Printf(TEXT("%.2f"), SlotKV.Value->OpenskillRating.GetSigma()))
                     .LeftPad(11),
                *(!SlotKV.Value->AssignedUserId.IsValid()
                      ? TEXT("")
                      : FString::Printf(TEXT("%.2f"), SlotKV.Value->OpenskillRating.GetOrdinal()))
                     .LeftPad(9),
                *UserId);
        }
    }
#endif
}

void FMatchmakingEngineTaskTeamManager::SetHostOpenskillRating(const FOpenskillRating &InRating)
{
    this->Teams[0][0]->OpenskillRating = InRating;
}

int32 FMatchmakingEngineTaskTeamManager::GetTotalCapacity() const
{
    return this->TotalCapacity;
}

int32 FMatchmakingEngineTaskTeamManager::GetAssignedMemberCountOfTeam(FMatchmakingTeamId InTeamId) const
{
    int32 AssignedCount = 0;
    for (const auto &SlotKV : this->Teams[InTeamId])
    {
        if (SlotKV.Value->AssignedUserId.IsValid())
        {
            AssignedCount++;
        }
    }
    UE_LOG(
        LogMatchmakingTeamManagement,
        Verbose,
        TEXT("%s: Assigned member count of team %d is %d"),
        *this->HostUserId->ToString(),
        InTeamId,
        AssignedCount);
    return AssignedCount;
}

int32 FMatchmakingEngineTaskTeamManager::GetUnassignedSlotCount() const
{
    int32 SlotCount = 0;
    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (!SlotKV.Value->AssignedUserId.IsValid())
            {
                SlotCount++;
            }
        }
    }
    if (this->HostUserId.IsValid())
    {
        UE_LOG(
            LogMatchmakingTeamManagement,
            Verbose,
            TEXT("%s: Unassigned slot count is %d"),
            *this->HostUserId->ToString(),
            SlotCount);
    }
    return SlotCount;
}

TArray<TArray<TSharedPtr<const FUniqueNetId>>> FMatchmakingEngineTaskTeamManager::ComputeTheoreticalTeamLayout(
    const TUserIdMap<FMatchmakingTeamId> &TheoreticalTeamReassignments) const
{
    TArray<TArray<TSharedPtr<const FUniqueNetId>>> Results;
    for (int32 i = 0; i < this->Teams.Num(); i++)
    {
        TArray<TSharedPtr<const FUniqueNetId>> TeamUsers;
        for (const auto &SlotKV : this->Teams[i])
        {
            if (SlotKV.Value->AssignedUserId.IsValid())
            {
                TeamUsers.Add(SlotKV.Value->AssignedUserId);
            }
        }
        Results.Add(TeamUsers);
    }
    for (const auto &Reassignment : TheoreticalTeamReassignments)
    {
        Results[Reassignment.Value].Add(Reassignment.Key);
    }
    return Results;
}

FOpenskillRating FMatchmakingEngineTaskTeamManager::GetUserOpenskillRating(const FUniqueNetId &InUserId) const
{
    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (SlotKV.Value->AssignedUserId.IsValid() && InUserId == *SlotKV.Value->AssignedUserId)
            {
                return SlotKV.Value->OpenskillRating;
            }
        }
    }

    checkf(false, TEXT("User must be assigned to a team in order to call GetUserOpenskillRating!"));
    return FOpenskillRating();
}

double FMatchmakingEngineTaskTeamManager::GetMatchAverageOpenskillOrdinal() const
{
    double AvgTotal = 0.0;
    int32 AvgCount = 0;
    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (SlotKV.Value->AssignedUserId.IsValid())
            {
                AvgTotal += SlotKV.Value->OpenskillRating.GetOrdinal();
                AvgCount++;
            }
        }
    }

    if (AvgCount == 0)
    {
        return FOpenskillRating().GetOrdinal();
    }
    else
    {
        return AvgTotal / AvgCount;
    }
}

bool FMatchmakingEngineTaskTeamManager::TryAddPendingBatch(
    const TUserIdMap<FMatchmakingTeamId> &InPendingUsers,
    const TUserIdMap<FOpenskillRating> &InSkillRatings)
{
    // Check if any of the users are already assigned in the team manager. If they are, ensure it matches
    // the teams we want to assign them to.
    TUserIdMap<FMatchmakingTeamId> DeduplicatedPendingUsers;
    for (const auto &TeamAssignment : InPendingUsers)
    {
        bool bAlreadyOnTeam = false;
        for (const auto &TeamKV : this->Teams)
        {
            for (const auto &SlotKV : TeamKV.Value)
            {
                if (SlotKV.Value->AssignedUserId.IsValid() && *SlotKV.Value->AssignedUserId == *TeamAssignment.Key)
                {
                    if (TeamAssignment.Value != TeamKV.Key)
                    {
                        // Asking to assign a user to a different team than the one they're already on.
                        this->DumpTeamManagerState();
                        UE_LOG(
                            LogMatchmaking,
                            Warning,
                            TEXT("Tried to assign user (%s) to a different team (%d) than the one they're already on "
                                 "(%d)."),
                            *TeamAssignment.Key->ToString(),
                            TeamAssignment.Value,
                            TeamKV.Key);
                        return false;
                    }
                    else
                    {
                        // This user is already on the team, so we don't need to add them again.
                        bAlreadyOnTeam = true;
                    }
                }
            }
        }
        if (!bAlreadyOnTeam)
        {
            DeduplicatedPendingUsers.Add(*TeamAssignment.Key, TeamAssignment.Value);
        }
    }

    // Now check capacities - make sure that all of the new users can actually fit in
    // the teams they're requesting.
    for (const auto &TeamKV : this->Teams)
    {
        int32 RequiredSlots = 0;
        for (const auto &TeamAssignment : DeduplicatedPendingUsers)
        {
            if (TeamAssignment.Value == TeamKV.Key)
            {
                RequiredSlots++;
            }
        }

        int32 AvailableSlots = 0;
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (!SlotKV.Value->AssignedUserId.IsValid())
            {
                AvailableSlots++;
            }
        }

        if (AvailableSlots < RequiredSlots)
        {
            // We don't have enough room.
            this->DumpTeamManagerState();
            UE_LOG(LogMatchmaking, Warning, TEXT("Did not have enough room to assign pending users to teams."));
            return false;
        }
    }

    // We've checked available and everything is OK, now assign the users
    // to the teams.
    for (const auto &TeamAssignment : DeduplicatedPendingUsers)
    {
#if DO_CHECK
        bool bDidAssign = false;
#endif
        for (const auto &SlotKV : this->Teams[TeamAssignment.Value])
        {
            if (!SlotKV.Value->AssignedUserId.IsValid())
            {
                SlotKV.Value->AssignedUserId = TeamAssignment.Key;
                SlotKV.Value->bIsInPendingBatch = true;
                SlotKV.Value->bConnected = false;
                if (InSkillRatings.Contains(*TeamAssignment.Key))
                {
                    SlotKV.Value->OpenskillRating = InSkillRatings[*TeamAssignment.Key];
                }
                else
                {
                    SlotKV.Value->OpenskillRating = FOpenskillRating();
                }
#if DO_CHECK
                bDidAssign = true;
#endif
                break;
            }
        }
#if DO_CHECK
        checkf(bDidAssign, TEXT("Must have assigned user to team after validation!"));
#endif
    }

    UE_LOG(
        LogMatchmakingTeamManagement,
        Verbose,
        TEXT("%s: Successfully added pending batch"),
        *this->HostUserId->ToString());

    this->bHasPendingBatch = true;
    this->DumpTeamManagerState();
    return true;
}

void FMatchmakingEngineTaskTeamManager::RemovePendingBatch(
    TArray<TSharedRef<const FUniqueNetId>> &OutConnectedUsersToKick)
{
    checkf(this->HasPendingBatch(), TEXT("Must have pending batch in order to call FinalizePendingBatch"));

    UE_LOG(LogMatchmakingTeamManagement, Verbose, TEXT("%s: Removed pending batch"), *this->HostUserId->ToString());

    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (SlotKV.Value->bIsInPendingBatch)
            {
                checkf(SlotKV.Value->AssignedUserId.IsValid(), TEXT("Slot must have assigned user ID if it's pending"));

                if (SlotKV.Value->bConnected)
                {
                    OutConnectedUsersToKick.Add(SlotKV.Value->AssignedUserId.ToSharedRef());
                }

                SlotKV.Value->AssignedUserId.Reset();
                SlotKV.Value->bIsInPendingBatch = false;
                SlotKV.Value->bConnected = false;
            }
        }
    }

    this->bHasPendingBatch = false;
    this->DumpTeamManagerState();
}

bool FMatchmakingEngineTaskTeamManager::HasPendingBatch() const
{
    return this->bHasPendingBatch;
}

bool FMatchmakingEngineTaskTeamManager::IsPendingBatchReady() const
{
    checkf(this->bHasPendingBatch, TEXT("IsPendingBatchReady can only be called if there is a pending batch"));

    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (SlotKV.Value->bIsInPendingBatch && !SlotKV.Value->bConnected)
            {
                return false;
            }
        }
    }

    return true;
}

void FMatchmakingEngineTaskTeamManager::FinalizePendingBatch()
{
    checkf(this->HasPendingBatch(), TEXT("Must have pending batch in order to call FinalizePendingBatch"));

    UE_LOG(LogMatchmakingTeamManagement, Verbose, TEXT("%s: Finalized pending batch"), *this->HostUserId->ToString());

    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (SlotKV.Value->bIsInPendingBatch)
            {
                checkf(SlotKV.Value->AssignedUserId.IsValid(), TEXT("Slot must have assigned user ID if it's pending"));
                checkf(SlotKV.Value->bConnected, TEXT("Slot must be connected in order to FinalizePendingBatch"));

                SlotKV.Value->bIsInPendingBatch = false;
            }
        }
    }

    this->bHasPendingBatch = false;
    this->DumpTeamManagerState();
}

TArray<TSharedRef<const FUniqueNetId>> FMatchmakingEngineTaskTeamManager::GetPendingNotConnectedUsers() const
{
    TArray<TSharedRef<const FUniqueNetId>> Results;

    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (SlotKV.Value->bIsInPendingBatch && !SlotKV.Value->bConnected)
            {
                checkf(SlotKV.Value->AssignedUserId.IsValid(), TEXT("Slot must have assigned user ID if it's pending"));

                Results.Add(SlotKV.Value->AssignedUserId.ToSharedRef());
            }
        }
    }

    return Results;
}

bool FMatchmakingEngineTaskTeamManager::TryMarkAsConnected(const FUniqueNetId &InUserId)
{
    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (SlotKV.Value->AssignedUserId.IsValid() && InUserId == *SlotKV.Value->AssignedUserId)
            {
                UE_LOG(
                    LogMatchmakingTeamManagement,
                    Verbose,
                    TEXT("%s: Marked %s as connected"),
                    *this->HostUserId->ToString(),
                    *SlotKV.Value->AssignedUserId->ToString());
                SlotKV.Value->bConnected = true;
                this->DumpTeamManagerState();
                return true;
            }
        }
    }

    this->DumpTeamManagerState();
    return false;
}

void FMatchmakingEngineTaskTeamManager::MarkAsDisconnected(const FUniqueNetId &InUserId)
{
    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (SlotKV.Value->AssignedUserId.IsValid() && InUserId == *SlotKV.Value->AssignedUserId)
            {
                UE_LOG(
                    LogMatchmakingTeamManagement,
                    Verbose,
                    TEXT("%s: Marked %s as disconnected"),
                    *this->HostUserId->ToString(),
                    *SlotKV.Value->AssignedUserId->ToString());
                SlotKV.Value->bConnected = false;
                this->DumpTeamManagerState();
                return;
            }
        }
    }

    this->DumpTeamManagerState();
}

bool FMatchmakingEngineTaskTeamManager::IsConnected(const FUniqueNetId &InUserId) const
{
    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (SlotKV.Value->AssignedUserId.IsValid() && InUserId == *SlotKV.Value->AssignedUserId)
            {
                return SlotKV.Value->bConnected;
            }
        }
    }
    return false;
}

bool FMatchmakingEngineTaskTeamManager::IsMatchComplete() const
{
    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (!SlotKV.Value->AssignedUserId.IsValid() || SlotKV.Value->bIsInPendingBatch || !SlotKV.Value->bConnected)
            {
                return false;
            }
        }
    }

    return true;
}

TUserIdMap<FMatchmakingTeamId> FMatchmakingEngineTaskTeamManager::GetFinalTeams() const
{
    TUserIdMap<FMatchmakingTeamId> FinalTeams;
    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (SlotKV.Value->AssignedUserId.IsValid())
            {
                FinalTeams.Add(*SlotKV.Value->AssignedUserId, TeamKV.Key);
            }
        }
    }
    this->DumpTeamManagerState();
    return FinalTeams;
}

FMatchmakingTeamId FMatchmakingEngineTaskTeamManager::GetAssignedTeam(const FUniqueNetId &InUserId)
{
    for (const auto &TeamKV : this->Teams)
    {
        for (const auto &SlotKV : TeamKV.Value)
        {
            if (SlotKV.Value->AssignedUserId.IsValid() && InUserId == *SlotKV.Value->AssignedUserId)
            {
                return TeamKV.Key;
            }
        }
    }

    return EMatchmakingTeamId_None;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()