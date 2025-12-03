// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/CreateDeviceIdNode.h"

#include "RedpointEOSAPI/Connect/CreateDeviceId.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1135899555, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FCreateDeviceIdNode::Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Connect;
    
    FCreateDeviceId::Execute(
        State->PlatformHandle,
        FCreateDeviceId::Options{"EOS Anonymous Login"},
        FCreateDeviceId::CompletionDelegate::CreateSPLambda(
            this,
            [State, OnDone](const FCreateDeviceId::Result &Data) {
                if (Data.ResultCode == EOS_EResult::EOS_Success ||
                    Data.ResultCode == EOS_EResult::EOS_DuplicateNotAllowed)
                {
                    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
                    return;
                }

                State->ErrorMessages.Add(FString::Printf(
                    TEXT("Unable to create Device Id, got result code %s"),
                    ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode))));
                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
                return;
            }));
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()