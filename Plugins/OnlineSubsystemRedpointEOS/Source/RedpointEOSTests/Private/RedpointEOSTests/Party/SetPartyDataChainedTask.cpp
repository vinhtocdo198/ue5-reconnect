// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/SetPartyDataChainedTask.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3938274349, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

FSetPartyDataChainedTask::FSetPartyDataChainedTask(
    int32 InInstanceId,
    TSharedRef<TSharedPtr<const FOnlinePartyId>> InStoredPartyId,
    FModifyPartyData InModifyPartyData,
    FVerifyPartyData InVerifyPartyData,
    bool bInIsMemberData)
    : InstanceId(InInstanceId)
    , StoredPartyId(InStoredPartyId)
    , ModifyPartyData(InModifyPartyData)
    , VerifyPartyData(InVerifyPartyData)
    , bIsMemberData(bInIsMemberData)
    , EventHandle()
    , TimeoutHandle()
    , bHandled(false)
{
}

void FSetPartyDataChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    this->TimeoutHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(this, &FSetPartyDataChainedTask::OnTimeout, Context, OnDone),
        20.f);

    if (this->bIsMemberData)
    {
        this->EventHandle =
            Context->Instance(this->InstanceId)
                .GetPartyInterface()
                ->AddOnPartyMemberDataReceivedDelegate_Handle(FOnPartyMemberDataReceived::FDelegate::CreateSP(
                    this,
                    &FSetPartyDataChainedTask::OnPartyMemberDataReceived,
                    Context,
                    OnDone));
        auto NewData = MakeShared<FOnlinePartyData>(*Context->Instance(this->InstanceId)
                                                         .GetPartyInterface()
                                                         ->GetPartyMemberData(
                                                             Context->User(this->InstanceId),
                                                             **this->StoredPartyId,
                                                             Context->User(this->InstanceId),
                                                             DefaultPartyDataNamespace));
        this->ModifyPartyData.ExecuteIfBound(*NewData);
        Context->Instance(this->InstanceId)
            .GetPartyInterface()
            ->UpdatePartyMemberData(
                Context->User(this->InstanceId),
                **this->StoredPartyId,
                DefaultPartyDataNamespace,
                *NewData);
    }
    else
    {
        this->EventHandle = Context->Instance(this->InstanceId)
                                .GetPartyInterface()
                                ->AddOnPartyDataReceivedDelegate_Handle(FOnPartyDataReceived::FDelegate::CreateSP(
                                    this,
                                    &FSetPartyDataChainedTask::OnPartyDataReceived,
                                    Context,
                                    OnDone));
        auto NewData = MakeShared<FOnlinePartyData>(
            *Context->Instance(this->InstanceId)
                 .GetPartyInterface()
                 ->GetPartyData(Context->User(this->InstanceId), **this->StoredPartyId, DefaultPartyDataNamespace));
        this->ModifyPartyData.ExecuteIfBound(*NewData);
        Context->Instance(this->InstanceId)
            .GetPartyInterface()
            ->UpdatePartyData(
                Context->User(this->InstanceId),
                **this->StoredPartyId,
                DefaultPartyDataNamespace,
                *NewData);
    }
}

bool FSetPartyDataChainedTask::OnTimeout(float DeltaSeconds, FChainedTaskContextRef Context, FOnComplete OnDone)
{
    if (this->bHandled)
    {
        return false;
    }

    Context->Assert().TestTrue(
        FString::Printf(
            TEXT("Got timeout while waiting for %s."),
            this->bIsMemberData ? TEXT("party member data") : TEXT("party data")),
        false);
    Context->Instance(this->InstanceId).GetPartyInterface()->ClearOnPartyDataReceivedDelegate_Handle(this->EventHandle);
    OnDone.ExecuteIfBound(false);
    return false;
}

void FSetPartyDataChainedTask::OnPartyDataReceived(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FName &Namespace,
    const FOnlinePartyData &PartyData,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (LocalUserId == Context->User(this->InstanceId) && PartyId == **this->StoredPartyId &&
        Namespace == DefaultPartyDataNamespace)
    {
        this->bHandled = true;
        FTSTicker::GetCoreTicker().RemoveTicker(this->TimeoutHandle);
        Context->Instance(this->InstanceId)
            .GetPartyInterface()
            ->ClearOnPartyDataReceivedDelegate_Handle(this->EventHandle);
        this->VerifyPartyData.ExecuteIfBound(Context, PartyData.AsShared());
        OnDone.ExecuteIfBound(true);
    }
}

void FSetPartyDataChainedTask::OnPartyMemberDataReceived(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    const FName &Namespace,
    const FOnlinePartyData &PartyData,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (LocalUserId == Context->User(this->InstanceId) && PartyId == **this->StoredPartyId &&
        Namespace == DefaultPartyDataNamespace && MemberId == Context->User(this->InstanceId))
    {
        this->bHandled = true;
        FTSTicker::GetCoreTicker().RemoveTicker(this->TimeoutHandle);
        Context->Instance(this->InstanceId)
            .GetPartyInterface()
            ->ClearOnPartyMemberDataReceivedDelegate_Handle(this->EventHandle);
        this->VerifyPartyData.ExecuteIfBound(Context, PartyData.AsShared());
        OnDone.ExecuteIfBound(true);
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()