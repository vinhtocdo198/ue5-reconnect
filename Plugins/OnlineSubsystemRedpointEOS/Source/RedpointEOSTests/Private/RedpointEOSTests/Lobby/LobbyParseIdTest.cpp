// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2971226777, Redpoint::EOS::Tests::Lobby)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Lobby_ParseId,
    "Redpoint.EOS.Lobby.ParseId",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Lobby_ParseId::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}));

    // Validate that lobby parsing works as expected.
    Executor->Then<FLambdaChainedTask>(
        [](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            auto LobbyInterface = Online::GetLobbyInterface(&Context->Instance(0));
            if (Context->Assert().TestTrue(TEXT("LobbyInterface is valid"), LobbyInterface.IsValid()))
            {
                auto Plain = LobbyInterface->ParseSerializedLobbyId(TEXT("0609837409514615b9bca4db150c2efe"));
                if (Context->Assert().TestTrue(TEXT("ParseSerializedLobbyId plain valid"), Plain.IsValid()))
                {
                    Context->Assert().TestEqual(
                        TEXT("ParseSerializedLobbyId plain equal"),
                        Plain->ToString(),
                        TEXT("0609837409514615b9bca4db150c2efe"));
                }

                auto Prefixed = LobbyInterface->ParseSerializedLobbyId(TEXT("Lobby:0609837409514615b9bca4db150c2efe"));
                if (Context->Assert().TestTrue(TEXT("ParseSerializedLobbyId prefixed valid"), Plain.IsValid()))
                {
                    Context->Assert().TestEqual(
                        TEXT("ParseSerializedLobbyId prefixed equal"),
                        Plain->ToString(),
                        TEXT("0609837409514615b9bca4db150c2efe"));
                }
            }
            TaskOnDone.ExecuteIfBound(true);
        });

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()