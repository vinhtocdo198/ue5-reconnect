// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/TitleFile/ReadTitleFileChainedTask.h"

#include "Interfaces/OnlineTitleFileInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(116812569, Redpoint::EOS::Tests::TitleFile)
{
using namespace ::Redpoint::EOS::Tests::TitleFile;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

static const char NS_TF_TestFile_Small[] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};
static const int32 NS_TF_TestFile_SmallSize = (int32)sizeof(NS_TF_TestFile_Small);

FReadTitleFileChainedTask::FReadTitleFileChainedTask(int32 InInstanceId, FString InFilename)
    : InstanceId(InInstanceId)
    , Filename(InFilename)
{
}

void FReadTitleFileChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    auto TitleFileInterface = Context->Instance(this->InstanceId).GetTitleFileInterface();

    // Register event for reading file.
    this->OnReadFileHandle = TitleFileInterface->AddOnReadFileCompleteDelegate_Handle(
        FOnReadFileCompleteDelegate::CreateSP(this, &FReadTitleFileChainedTask::OnReadFileComplete, Context, OnDone));

    // Start reading the file.
    if (!Context->Assert().TestTrue(
            TEXT("Expected ReadFile to return true."),
            TitleFileInterface->ReadFile(this->Filename)))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
}

void FReadTitleFileChainedTask::OnReadFileComplete(
    bool bWasSuccessful,
    const FString &,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    auto TitleFileInterface = Context->Instance(this->InstanceId).GetTitleFileInterface();

    TitleFileInterface->ClearOnReadFileCompleteDelegate_Handle(this->OnReadFileHandle);

    if (!Context->Assert().TestTrue(TEXT("Expected ReadFile operation to succeed."), bWasSuccessful))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    TArray<uint8> FileContentsRead;
    if (!Context->Assert().TestTrue(
            TEXT("Expected GetFileContents to succeed."),
            TitleFileInterface->GetFileContents(this->Filename, FileContentsRead)))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    Context->Assert().TestEqual("File has expected length", FileContentsRead.Num(), NS_TF_TestFile_SmallSize);
    if (FileContentsRead.Num() == NS_TF_TestFile_SmallSize)
    {
        for (int32 i = 0; i < NS_TF_TestFile_SmallSize; i++)
        {
            Context->Assert().TestEqual(
                FString::Printf(TEXT("File byte at index %d matches"), i),
                FileContentsRead[i],
                NS_TF_TestFile_Small[i]);
        }
    }

    OnDone.ExecuteIfBound(true);
    return;
}

}

REDPOINT_EOS_CODE_GUARD_END()