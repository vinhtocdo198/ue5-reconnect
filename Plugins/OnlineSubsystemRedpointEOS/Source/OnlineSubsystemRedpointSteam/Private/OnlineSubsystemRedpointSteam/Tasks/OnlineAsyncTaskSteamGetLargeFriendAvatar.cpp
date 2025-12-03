// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamGetLargeFriendAvatar.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointSteam/Inventory/SteamInventoryConstants.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"
#include <limits>

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(500868213, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam;

FOnlineAsyncTaskSteamGetLargeFriendAvatar::FOnlineAsyncTaskSteamGetLargeFriendAvatar(
    uint64 InSteamID,
    FSteamAvatarDataFetched InDelegate)
    : bInit(false)
    , SteamID(InSteamID)
    , Delegate(MoveTemp(InDelegate))
    , ResultWidth()
    , ResultHeight()
    , ResultRGBA()
    , ResultError()
    , bHasChecked(false)
    , bWasImmediatelyAvailable(false)
{
}

void FOnlineAsyncTaskSteamGetLargeFriendAvatar::Tick()
{
    if (!this->bInit)
    {
        this->bInit = true;
    }

    int Picture = SteamFriends()->GetLargeFriendAvatar(this->SteamID);
    if (Picture == -1)
    {
        // @note: AvatarImageLoaded_t does not seem reliable when we set up
        // an event listening in response to -1, so instead we just continously
        // retry calling GetLargeFriendAvatar until it returns something.
        this->bHasChecked = true;
        return;
    }
    else if (Picture == 0)
    {
        // This user has no avatar set.
        this->bIsComplete = true;
        this->bWasSuccessful = false;
        this->ResultError = OnlineRedpointEOS::Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(TEXT("Steam ID %llu has no avatar set"), this->SteamID));
        return;
    }

    // The avatar is ready.
    if (!this->bHasChecked)
    {
        this->bWasImmediatelyAvailable = true;
    }

    uint32 Width = 0;
    uint32 Height = 0;
    SteamUtils()->GetImageSize(Picture, &Width, &Height);

    size_t BufferSize = (size_t)Width * Height * 4;
    if (Width > 0 && Height > 0 && BufferSize < (size_t)std::numeric_limits<int>::max())
    {
        uint8 *AvatarRGBA = (uint8 *)FMemory::MallocZeroed(BufferSize);
        SteamUtils()->GetImageRGBA(Picture, AvatarRGBA, BufferSize);
        for (uint32 i = 0; i < (Width * Height * 4); i += 4)
        {
            uint8 Temp = AvatarRGBA[i + 0];
            AvatarRGBA[i + 0] = AvatarRGBA[i + 2];
            AvatarRGBA[i + 2] = Temp;
        }

        this->ResultWidth = Width;
        this->ResultHeight = Height;
        this->ResultRGBA = TArray<uint8>(AvatarRGBA, BufferSize);
        this->bWasSuccessful = true;

        FMemory::Free(AvatarRGBA);
    }
    else
    {
        this->bWasSuccessful = false;
        this->ResultError = OnlineRedpointEOS::Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The specified avatar has an invalid size."));
    }

    this->bIsComplete = true;
}

void FOnlineAsyncTaskSteamGetLargeFriendAvatar::Finalize()
{
    if (!this->bWasSuccessful)
    {
        UE_LOG(LogRedpointSteam, Error, TEXT("%s"), *this->ResultError.ToLogString());
        this->Delegate.ExecuteIfBound(this->ResultError, 0, 0, TArray<uint8>());
    }
    else
    {
        if (this->bWasImmediatelyAvailable)
        {
            UE_LOG(LogRedpointSteam, VeryVerbose, TEXT("Steam ID %llu avatar is immediately ready"), this->SteamID);
        }

        this->Delegate.ExecuteIfBound(
            OnlineRedpointEOS::Errors::Success(),
            this->ResultWidth,
            this->ResultHeight,
            this->ResultRGBA);
    }
}

}

#endif