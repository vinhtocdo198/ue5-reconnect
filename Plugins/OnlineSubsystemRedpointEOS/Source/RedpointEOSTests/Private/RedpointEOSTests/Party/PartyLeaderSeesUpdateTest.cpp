// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSTests/Party/CreatePartyChainedTask.h"
#include "RedpointEOSTests/Party/SetPartyDataChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskExecutor.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(358988961, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Tests::Party;

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Party_LeaderSeesUpdate,
    "Redpoint.EOS.Party.LeaderSeesUpdate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Party_LeaderSeesUpdate::RunAsyncTest(const TFunction<void()> &OnDone)
{
    using namespace ::Redpoint::EOS::Tests::TestUtilities;
    using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

    TSharedRef<FChainedTaskExecutor> Executor = MakeShareable(new FChainedTaskExecutor(this, OnDone, TArray<int32>{1}));

    // Create shared pointers to hold data.
    TSharedRef<TSharedPtr<const FOnlinePartyId>> StoredPartyId = MakeShared<TSharedPtr<const FOnlinePartyId>>();

    // Create a party.
    Executor->Then<FCreatePartyChainedTask>(0, StoredPartyId);

    // Update the party data on the host.
    Executor->Then<FSetPartyDataChainedTask>(
        0,
        StoredPartyId,
        FSetPartyDataChainedTask::FModifyPartyData::CreateLambda([](FOnlinePartyData &PartyData) {
            PartyData.SetAttribute(TEXT("MyAttributeBool"), FVariantData((bool)true));
        }),
        FSetPartyDataChainedTask::FVerifyPartyData::CreateLambda(
            [](const FChainedTaskContextRef &Context, const FOnlinePartyDataConstPtr &PartyData) {
                FString AttributeKey = TEXT("MyAttributeBool");
                FVariantData Attr;
                if (Context->Assert().TestTrue(
                        FString::Printf(TEXT("Attribute %s is present"), *AttributeKey),
                        PartyData->GetAttribute(AttributeKey, Attr)))
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
            }),
        false);

    // Update the party data on the host a second time. Don't make any real changes to
    // the party data state, but just verify that the event fires.
    Executor->Then<FSetPartyDataChainedTask>(
        0,
        StoredPartyId,
        FSetPartyDataChainedTask::FModifyPartyData::CreateLambda([](FOnlinePartyData &PartyData) {
            PartyData.SetAttribute(TEXT("MyAttributeBool"), FVariantData((bool)true));
        }),
        FSetPartyDataChainedTask::FVerifyPartyData::CreateLambda(
            [](const FChainedTaskContextRef &Context, const FOnlinePartyDataConstPtr &PartyData) {
                FString AttributeKey = TEXT("MyAttributeBool");
                FVariantData Attr;
                if (Context->Assert().TestTrue(
                        FString::Printf(TEXT("Attribute %s is present"), *AttributeKey),
                        PartyData->GetAttribute(AttributeKey, Attr)))
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
            }),
        false);

    // Run the test.
    Executor->Execute();
}

}

REDPOINT_EOS_CODE_GUARD_END()