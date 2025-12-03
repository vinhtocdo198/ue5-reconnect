// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/DeletePersistentAuthNode.h"

#include "RedpointEOSAPI/Auth/DeletePersistentAuth.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSCore/Module.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3012757551, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

void FDeletePersistentAuthNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Auth;

    Redpoint::EOS::Core::FModule::GetModuleChecked().GetRuntimePlatform()->ClearStoredEASRefreshToken(
        State->LocalUserNum);

    FDeletePersistentAuth::Execute(
        State->PlatformHandle,
        FDeletePersistentAuth::Options{TEXT("")},
        FDeletePersistentAuth::CompletionDelegate::CreateSPLambda(
            this,
            [OnDone](const FDeletePersistentAuth::Result &Result) {
                if (Result.ResultCode != EOS_EResult::EOS_Success)
                {
                    UE_LOG(
                        LogRedpointEOSAuth,
                        Warning,
                        TEXT("Could not delete persistent authentication token on logout: %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Result.ResultCode)));
                }

                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()