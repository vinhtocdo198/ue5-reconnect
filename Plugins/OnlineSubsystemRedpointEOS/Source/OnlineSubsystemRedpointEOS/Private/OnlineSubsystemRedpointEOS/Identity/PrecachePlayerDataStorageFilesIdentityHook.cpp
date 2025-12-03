// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Identity/PrecachePlayerDataStorageFilesIdentityHook.h"

#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/UserCloud/OnlineUserCloudInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "OnlineSubsystemRedpointEOSModule.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3758799363, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::UserCloud;

void FPrecachePlayerDataStorageFilesIdentityHook::OnStartSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStartSystemComplete &InOnComplete)
{
    using namespace ::Redpoint::EOS::API;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Storage;

    if (InContext.Environment->bIsDedicatedServer)
    {
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    TArray<FString> PrecacheList = InContext.Environment->Config->GetPrecachePlayerDataStorageFiles();
    if (PrecacheList.Num() == 0)
    {
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    auto OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemRedpointEOSModule>(TEXT("OnlineSubsystemRedpointEOS"))
                   .GetExistingSubsystem(InContext.Environment->InstanceName);
    if (!OSS.IsValid())
    {
        InOnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to locate world to precache Player Data Storage files of signed in user.")));
        return;
    }

    auto UserCloud = StaticCastSharedPtr<FOnlineUserCloudInterfaceEOS>(OSS->GetUserCloudInterface());

    FMultiOperation<FString, bool>::RunSP(
        this,
        PrecacheList,
        [UserCloud, InUser](FString FileName, TFunction<void(bool)> OutDone) -> bool {
            UserCloud->ReadUserFileToCache(
                InUser->GetUserId(),
                FileName,
                IStorageSystem::FOnReadComplete::CreateLambda(
                    [FileName, OutDone](const FError &ErrorCode, const TArray<uint8> &FileData) {
                        if (!ErrorCode.WasSuccessful())
                        {
                            UE_LOG(
                                LogOnlineSubsystemRedpointEOS,
                                Warning,
                                TEXT("Unable to precache Player Data Storage file '%s': %s"),
                                *FileName,
                                *ErrorCode.ToLogString());
                        }
                        OutDone(true);
                    }));
            return true;
        },
        [InOnComplete](TArray<bool> OutValues) {
            InOnComplete.ExecuteIfBound(Errors::Success());
        });
}

}

#endif