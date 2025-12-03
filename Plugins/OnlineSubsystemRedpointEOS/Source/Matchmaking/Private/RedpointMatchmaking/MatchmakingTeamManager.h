// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingEngine.h"
#include "RedpointMatchmaking/Openskill/OpenskillRating.h"
#include "RedpointMatchmaking/Utility/UserIdContainers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

typedef int32 FMatchmakingTeamId;
typedef int32 FMatchmakingSlotId;
const FMatchmakingTeamId EMatchmakingTeamId_None = -1;

struct FMatchmakingSlotInfo
{
public:
    FMatchmakingSlotInfo()
        : AssignedUserId(nullptr)
        , bIsInPendingBatch(false)
        , bConnected(false)
        , OpenskillRating()
    {
    }

    /**
     * The user that is assigned to this slot.
     */
    TSharedPtr<const FUniqueNetId> AssignedUserId;

    /**
     * If true, the user is part of the "pending" batch. The team manager currently only supports
     * one pending batch at a time. If you cancel the pending batch, all of the pending batch users
     * are removed from the team manager, and then kicked if they are still connected to the
     * lobby.
     */
    bool bIsInPendingBatch;

    /**
     * If true, the user is connected.
     */
    bool bConnected;

    /**
     * The Openskill rating of the user, if skill-based matchmaking is enabled and this slot is
     * assigned a user.
     */
    FOpenskillRating OpenskillRating;
};

class FMatchmakingEngineTaskTeamManager : public TSharedFromThis<FMatchmakingEngineTaskTeamManager>
{
private:
    TSharedPtr<const FUniqueNetId> HostUserId;
    bool bHasPendingBatch;
    int32 TotalCapacity;
    TMap<FMatchmakingTeamId, TMap<FMatchmakingSlotId, TSharedRef<FMatchmakingSlotInfo>>> Teams;

    void SetupInitialState(const FMatchmakingEngineRequest &InRequest);
    void DumpTeamManagerState() const;

public:
    FMatchmakingEngineTaskTeamManager(const FMatchmakingEngineRequest &InRequest);
    UE_NONCOPYABLE(FMatchmakingEngineTaskTeamManager);

    /**
     * Sets the Openskill rating for the host user. Since loading stats requires an asynchronous QueryStats
     * call, this is invoked from the LockParty step.
     */
    void SetHostOpenskillRating(const FOpenskillRating &InRating);

    /**
     * Returns the total capacity required for the matchmaking lobby (the total number of slots we have).
     */
    int32 GetTotalCapacity() const;

    /**
     * Returns the number of slots that are currently assigned to users on a given team.
     */
    int32 GetAssignedMemberCountOfTeam(FMatchmakingTeamId InTeamId) const;

    /**
     * Returns the number of slots that are not yet assigned to a user.
     */
    int32 GetUnassignedSlotCount() const;

    /**
     * Based on the current assigned users in the team manager, compute the new team layout if the provided
     * team reassignments were executed on. This is used by skill-based matchmaking to view the team layout
     * if the candidate lobby was chosen, and to perform balance calculations on it.
     */
    TArray<TArray<TSharedPtr<const FUniqueNetId>>> ComputeTheoreticalTeamLayout(
        const TUserIdMap<FMatchmakingTeamId> &TheoreticalTeamReassignments) const;

    /**
     * Returns the Openskill rating for the assigned user if skill-based matchmaking is enabled. Asserts
     * if the user isn't an assigned user in any team.
     */
    FOpenskillRating GetUserOpenskillRating(const FUniqueNetId &InUserId) const;

    /**
     * Gets the average Openskill rating of the entire match. This is used for ordering searches, so that we find
     * candidiates that are close to our match rating already.
     */
    double GetMatchAverageOpenskillOrdinal() const;

    /**
     * Tries to add the pending batch of users to the team manager. Only successful if it
     * can fit all of the users in all of the teams. This function will not rearrange teams
     * to fit with the existing data; it is expected the caller knows which slots are already
     * filled and is trying to assign members to teams in such a way that everything fits.
     *
     * If successful, members are always assigned to the team index that was explicitly passed
     * to this function.
     *
     * Asserts that there is no current pending batch in progress. Any previous pending
     * batch must either have been removed with RemovePendingBatch or finalized with
     * FinalizePendingBatch.
     */
    bool TryAddPendingBatch(
        const TUserIdMap<FMatchmakingTeamId> &InPendingUsers,
        const TUserIdMap<FOpenskillRating> &InSkillRatings);

    /**
     * Removes the current pending batch from team manager. Returns an array of users that
     * the caller needs to kick from the matchmaking lobby.
     */
    void RemovePendingBatch(TArray<TSharedRef<const FUniqueNetId>> &OutConnectedUsersToKick);

    /**
     * Returns true if there is a pending batch in progress.
     */
    bool HasPendingBatch() const;

    /**
     * Returns true if all of the pending users have been marked as connected.
     */
    bool IsPendingBatchReady() const;

    /**
     * Marks all of the pending users as no longer pending; this effectively finalizes the batch.
     */
    void FinalizePendingBatch();

    /**
     * Returns an array of users from the pending batch that we are waiting to see a connection from.
     */
    TArray<TSharedRef<const FUniqueNetId>> GetPendingNotConnectedUsers() const;

    /**
     * Tries to mark the specified user as connected. If the specified user isn't assigned to
     * a team already, returns false and doesn't do anything. If the caller receives false, it
     * should kick the user from the matchmaking lobby.
     */
    bool TryMarkAsConnected(const FUniqueNetId &InUserId);

    /**
     * Marks the specified user as not connected. This works even if the user isn't part of
     * the pending batch.
     *
     * If they're in the pending batch, a disconnected user will prevent IsPendingBatchReady from
     * returning true.
     *
     * If they're not in the pending batch, they just get marked as disconnected.
     */
    void MarkAsDisconnected(const FUniqueNetId &InUserId);

    /**
     * Returns true if the user is considered connected in the team manager, regardless of their pending status.
     */
    bool IsConnected(const FUniqueNetId &InUserId) const;

    /**
     * Returns true if all of the slots are filled with connected users.
     */
    bool IsMatchComplete() const;

    /**
     * Returns the final mapping of users to their team assignments, for the completion of matchmaking.
     */
    TUserIdMap<FMatchmakingTeamId> GetFinalTeams() const;

    /**
     * Returns the assigned team for the specified user.
     */
    FMatchmakingTeamId GetAssignedTeam(const FUniqueNetId &InUserId);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()