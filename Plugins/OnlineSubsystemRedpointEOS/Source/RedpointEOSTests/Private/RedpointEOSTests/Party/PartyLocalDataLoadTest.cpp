// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/Party/JoinPartyViaInviteChainedTask.h"
#include "RedpointEOSTests/Party/LeavePartyChainedTask.h"
#include "RedpointEOSTests/Party/PartyMemberDataUpdateStateHandle.h"
#include "RedpointEOSTests/Party/RegisterPartyMemberDataUpdateChainedTask.h"
#include "RedpointEOSTests/Party/RejectAllPendingInvitesChainedTask.h"
#include "RedpointEOSTests/Party/SendAndReceiveInvitationChainedTask.h"
#include "RedpointEOSTests/Party/SetPartyDataChainedTask.h"
#include "RedpointEOSTests/Party/VerifyPartyMembershipChainedTask.h"
#include "RedpointEOSTests/Party/WaitForPartyMemberDataUpdateChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/LambdaChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/RegisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/UnregisterEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/WaitForEventChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(898524097, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::Party;
using namespace ::Redpoint::EOS::Tests::TestUtilities;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_LocalDataLoad,
    "Redpoint.EOS.Party.LocalDataLoad",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

typedef TDelegate<void(
    TSharedRef<const FUniqueNetId> LocalUserId,
    TSharedRef<const FOnlinePartyId> PartyId,
    FName Namespace,
    TSharedRef<const FOnlinePartyData> PartyData)>
    FOnPartyDataReceivedDelegateByRef;
typedef TDelegate<void(
    TSharedRef<const FUniqueNetId> LocalUserId,
    TSharedRef<const FOnlinePartyId> PartyId,
    TSharedRef<const FUniqueNetId> MemberId,
    FName Namespace,
    TSharedRef<const FOnlinePartyData> PartyData)>
    FOnPartyMemberDataReceivedDelegateByRef;

static void VerifyAttributes(
    const FChainedTaskContextRef &Context,
    const FOnlinePartyDataConstPtr &Data,
    FString AttributePrefix)
{
    {
        FString AttributeKey = FString::Printf(TEXT("%s"), *AttributePrefix);
        FVariantData Attr;
        if (Context->Assert().TestTrue(
                FString::Printf(TEXT("Attribute %s is present"), *AttributeKey),
                Data->GetAttribute(AttributeKey, Attr)))
        {
            if (Context->Assert().TestEqual(
                    FString::Printf(TEXT("Attribute %s is of expected type"), *AttributeKey),
                    Attr.GetType(),
                    EOnlineKeyValuePairDataType::String))
            {
                FString AttrValue;
                Attr.GetValue(AttrValue);
                Context->Assert().TestEqual(
                    FString::Printf(TEXT("Attribute %s value is correct"), *AttributeKey),
                    AttrValue,
                    TEXT("MyData"));
            }
        }
        else
        {
            UE_DEBUG_BREAK();
        }
    }

    {
        FString AttributeKey = FString::Printf(TEXT("%sInt"), *AttributePrefix);
        FVariantData Attr;
        if (Context->Assert().TestTrue(
                FString::Printf(TEXT("Attribute %s is present"), *AttributeKey),
                Data->GetAttribute(AttributeKey, Attr)))
        {
            if (Context->Assert().TestEqual(
                    FString::Printf(TEXT("Attribute %s is of expected type"), *AttributeKey),
                    Attr.GetType(),
                    EOnlineKeyValuePairDataType::Int64))
            {
                int64 AttrValue;
                Attr.GetValue(AttrValue);
                Context->Assert().TestEqual(
                    FString::Printf(TEXT("Attribute %s value is correct"), *AttributeKey),
                    AttrValue,
                    static_cast<int64>(10));
            }
        }
    }

    {
        FString AttributeKey = FString::Printf(TEXT("%sDouble"), *AttributePrefix);
        FVariantData Attr;
        if (Context->Assert().TestTrue(
                FString::Printf(TEXT("Attribute %s is present"), *AttributeKey),
                Data->GetAttribute(AttributeKey, Attr)))
        {
            if (Context->Assert().TestEqual(
                    FString::Printf(TEXT("Attribute %s is of expected type"), *AttributeKey),
                    Attr.GetType(),
                    EOnlineKeyValuePairDataType::Double))
            {
                double AttrValue;
                Attr.GetValue(AttrValue);
                Context->Assert().TestEqual(
                    FString::Printf(TEXT("Attribute %s value is correct"), *AttributeKey),
                    AttrValue,
                    10.0);
            }
        }
    }

    {
        FString AttributeKey = FString::Printf(TEXT("%sBool"), *AttributePrefix);
        FVariantData Attr;
        if (Context->Assert().TestTrue(
                FString::Printf(TEXT("Attribute %s is present"), *AttributeKey),
                Data->GetAttribute(AttributeKey, Attr)))
        {
            if (Context->Assert().TestEqual(
                    FString::Printf(TEXT("Attribute %s is of expected type"), *AttributeKey),
                    Attr.GetType(),
                    EOnlineKeyValuePairDataType::Bool))
            {
                bool AttrValue;
                Attr.GetValue(AttrValue);
                Context->Assert().TestEqual(
                    FString::Printf(TEXT("Attribute %s value is correct"), *AttributeKey),
                    AttrValue,
                    true);
            }
        }
    }
}

void FOnlineSubsystemEOS_Party_LocalDataLoad::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor =
        MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1, 1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId = MakeShared<TSharedPtr<const FOnlinePartyId>>();
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> StoredInvite =
        MakeShared<TSharedPtr<const IOnlinePartyJoinInfo>>();
    TSharedRef<FPartyMemberDataUpdateStateHandle> UpdateState = MakeShared<FPartyMemberDataUpdateStateHandle>();

    // Create a party.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId);

    // Set party data on the host.
    Executor->Then<FSetPartyDataChainedTask>(
        0,
        StoredPartyId,
        FSetPartyDataChainedTask::FModifyPartyData::CreateLambda([](FOnlinePartyData &PartyData) {
            PartyData.SetAttribute(TEXT("MyAttribute"), FVariantData(TEXT("MyData")));
            PartyData.SetAttribute(TEXT("MyAttributeInt"), FVariantData((int64)10));
            PartyData.SetAttribute(TEXT("MyAttributeDouble"), FVariantData((double)10.0));
            PartyData.SetAttribute(TEXT("MyAttributeBool"), FVariantData((bool)true));
        }),
        FSetPartyDataChainedTask::FVerifyPartyData::CreateStatic(&VerifyAttributes, FString(TEXT("MyAttribute"))),
        false);

    // Set party member data on the host.
    Executor->Then<FSetPartyDataChainedTask>(
        0,
        StoredPartyId,
        FSetPartyDataChainedTask::FModifyPartyData::CreateLambda([](FOnlinePartyData &PartyData) {
            PartyData.SetAttribute(TEXT("MyMemberAttribute"), FVariantData(TEXT("MyData")));
            PartyData.SetAttribute(TEXT("MyMemberAttributeInt"), FVariantData((int64)10));
            PartyData.SetAttribute(TEXT("MyMemberAttributeDouble"), FVariantData((double)10.0));
            PartyData.SetAttribute(TEXT("MyMemberAttributeBool"), FVariantData((bool)true));
        }),
        FSetPartyDataChainedTask::FVerifyPartyData::CreateStatic(&VerifyAttributes, FString(TEXT("MyMemberAttribute"))),
        true);

    // Send invites from the first user to the other users.
    Executor->Then<FSendAndReceiveInvitationChainedTask>(0, 1, StoredPartyId, StoredInvite);

    // Join the party using the invite.
    Executor->Then<FJoinPartyViaInviteChainedTask>(1, StoredInvite);

    // Verify the party data and host's party member data on the client.
    Executor->Then<FLambdaChainedTask>(
        [StoredPartyId](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            auto PartyData = Context->Instance(1).GetPartyInterface()->GetPartyData(
                Context->User(1),
                **StoredPartyId,
                DefaultPartyDataNamespace);
            auto HostPartyMemberData = Context->Instance(1).GetPartyInterface()->GetPartyMemberData(
                Context->User(1),
                **StoredPartyId,
                Context->User(0),
                DefaultPartyDataNamespace);

            VerifyAttributes(Context, PartyData, TEXT("MyAttribute"));
            VerifyAttributes(Context, HostPartyMemberData, TEXT("MyMemberAttribute"));

            TaskOnDone.ExecuteIfBound(true);
        });

    // Register to listen for the client's party data changing on the host.
    Executor->Then<FRegisterPartyMemberDataUpdateChainedTask>(0, 1, StoredPartyId, UpdateState);

    // Set the client's party member data.
    Executor->Then<FSetPartyDataChainedTask>(
        1,
        StoredPartyId,
        FSetPartyDataChainedTask::FModifyPartyData::CreateLambda([](FOnlinePartyData &PartyData) {
            PartyData.SetAttribute(TEXT("MyClientAttribute"), FVariantData(TEXT("MyData")));
            PartyData.SetAttribute(TEXT("MyClientAttributeInt"), FVariantData((int64)10));
            PartyData.SetAttribute(TEXT("MyClientAttributeDouble"), FVariantData((double)10.0));
            PartyData.SetAttribute(TEXT("MyClientAttributeBool"), FVariantData((bool)true));
        }),
        FSetPartyDataChainedTask::FVerifyPartyData::CreateStatic(&VerifyAttributes, FString(TEXT("MyClientAttribute"))),
        true);

    // Wait for the host to see the client's party member data change.
    Executor->Then<FWaitForPartyMemberDataUpdateChainedTask>(UpdateState);

    // Verify the client's party member data on the host.
    Executor->Then<FLambdaChainedTask>(
        [StoredPartyId](const FChainedTaskContextRef &Context, const FLambdaChainedTask::FOnComplete &TaskOnDone) {
            auto HostPartyMemberData = Context->Instance(0).GetPartyInterface()->GetPartyMemberData(
                Context->User(0),
                **StoredPartyId,
                Context->User(1),
                DefaultPartyDataNamespace);

            VerifyAttributes(Context, HostPartyMemberData, TEXT("MyClientAttribute"));

            TaskOnDone.ExecuteIfBound(true);
        });

    // Leave the party.
    Executor->Then<FLeavePartyChainedTask>(0, StoredPartyId);
    Executor->Then<FLeavePartyChainedTask>(1, StoredPartyId);

    // Check that the users are no longer in parties.
    Executor->Then<FVerifyPartyMembershipChainedTask>(0, FVerifyPartyMembershipChainedTask::FInitList{});
    Executor->Then<FVerifyPartyMembershipChainedTask>(1, FVerifyPartyMembershipChainedTask::FInitList{});

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()