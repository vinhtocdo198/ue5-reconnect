// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Misc/AutomationTest.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/NetworkingSystemImpl.h"
#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2363523850, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Socket;
using namespace ::Redpoint::EOS::Core::Id;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FNetworkingSystemImplTest,
    "Redpoint.EOS.Networking.NetworkingSystemImpl",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

bool FNetworkingSystemImplTest::RunTest(const FString &Parameters)
{
    TSharedRef<FNetworkingSystemImpl> ListenTracker = MakeShared<FNetworkingSystemImpl>();

    UE::Online::FAccountId UserId = TryParseAccountId("0002b3f8adfe41249fef5e8197c76fd9").GetValue();
    TSharedRef<const FInternetAddr> RegAddr1 =
        MakeShared<FInternetAddrRedpointEOS>(UserId, FString(TEXT("SKT1")), static_cast<uint8>(1u));
    TSharedRef<const FInternetAddr> RegAddr2 =
        MakeShared<FInternetAddrRedpointEOS>(UserId, FString(TEXT("SKT2")), static_cast<uint8>(1u));

    // Get returns false with no registration.
    {
        TSharedPtr<const FInternetAddr> InternetAddr;
        TArray<TSharedRef<const FInternetAddr>> DeveloperAddrs;
        ListenTracker->GetListeningState(UserId, InternetAddr, DeveloperAddrs);
        TestFalse("No addresses for user", InternetAddr.IsValid());
    }

    // OnListeningStateChanged() fires on registration (SKT1 #1).
    {
        bool bDidFire = false;
        FDelegateHandle Delegate = ListenTracker->OnListeningStateChanged().AddLambda(
            [this, UserId, RegAddr1, &bDidFire](
                UE::Online::FAccountId EvProductUserId,
                const TSharedPtr<const FInternetAddr> &EvInternetAddr,
                const TArray<TSharedRef<const FInternetAddr>> &EvDeveloperInternetAddrs) {
                if (this->TestTrue("EvInternetAddr was valid", EvInternetAddr.IsValid()))
                {
                    this->TestEqual("Product user IDs equal OnListeningStateChanged()", UserId, EvProductUserId);
                    this->TestTrue("Internet addrs equal OnListeningStateChanged()", *EvInternetAddr == *RegAddr1);
                }
                bDidFire = true;
            });
        ListenTracker->AddListeningState(UserId, RegAddr1, TArray<TSharedRef<const FInternetAddr>>());
        TestTrue("OnListeningStateChanged() did fire", bDidFire);
        ListenTracker->OnListeningStateChanged().Remove(Delegate);
    }

    // Get returns true after registration (SKT1 #1).
    {
        TSharedPtr<const FInternetAddr> InternetAddr;
        TArray<TSharedRef<const FInternetAddr>> DeveloperAddrs;
        ListenTracker->GetListeningState(UserId, InternetAddr, DeveloperAddrs);
        if (TestTrue("Found address for user", InternetAddr.IsValid()))
        {
            TestTrue("Internet addrs equal Get", *InternetAddr == *RegAddr1);
        }
    }

    // OnListeningStateChanged() does *not* fire on registration (SKT1 #2).
    {
        bool bDidFire = false;
        FDelegateHandle Delegate = ListenTracker->OnListeningStateChanged().AddLambda(
            [=, this, &bDidFire](
                UE::Online::FAccountId EvProductUserId,
                const TSharedPtr<const FInternetAddr> &EvInternetAddr,
                const TArray<TSharedRef<const FInternetAddr>> &EvDeveloperInternetAddrs) {
                this->TestTrue("EvInternetAddr was valid", EvInternetAddr.IsValid());
                bDidFire = true;
            });
        ListenTracker->AddListeningState(UserId, RegAddr1, TArray<TSharedRef<const FInternetAddr>>());
        TestFalse("OnListeningStateChanged() did fire", bDidFire);
        ListenTracker->OnListeningStateChanged().Remove(Delegate);
    }

    // Get returns true after registration (SKT1 #2).
    {
        TSharedPtr<const FInternetAddr> InternetAddr;
        TArray<TSharedRef<const FInternetAddr>> DeveloperAddrs;
        ListenTracker->GetListeningState(UserId, InternetAddr, DeveloperAddrs);
        if (TestTrue("Found address for user", InternetAddr.IsValid()))
        {
            TestTrue("Internet addrs equal Get", *InternetAddr == *RegAddr1);
        }
    }

    // OnListeningStateChanged() does *not* fire on registration (SKT2 #1).
    {
        bool bDidFire = false;
        FDelegateHandle Delegate = ListenTracker->OnListeningStateChanged().AddLambda(
            [=, this, &bDidFire](
                UE::Online::FAccountId EvProductUserId,
                const TSharedPtr<const FInternetAddr> &EvInternetAddr,
                const TArray<TSharedRef<const FInternetAddr>> &EvDeveloperInternetAddrs) {
                this->TestTrue("EvInternetAddr was valid", EvInternetAddr.IsValid());
                bDidFire = true;
            });
        ListenTracker->AddListeningState(UserId, RegAddr2, TArray<TSharedRef<const FInternetAddr>>());
        TestFalse("OnListeningStateChanged() did not fire", bDidFire);
        ListenTracker->OnListeningStateChanged().Remove(Delegate);
    }

    // Get returns true after registration (SKT2 #1) and returns (SKT1).
    {
        TSharedPtr<const FInternetAddr> InternetAddr;
        TArray<TSharedRef<const FInternetAddr>> DeveloperAddrs;
        ListenTracker->GetListeningState(UserId, InternetAddr, DeveloperAddrs);
        if (TestTrue("Found address for user", InternetAddr.IsValid()))
        {
            TestTrue("Internet addrs equal Get", *InternetAddr == *RegAddr1);
        }
    }

    // OnListeningStateChanged() does *not* fire when RemoveListeningStateing (SKT1 #1), because SKT1 has two
    // references.
    {
        bool bDidFire = false;
        FDelegateHandle Delegate = ListenTracker->OnListeningStateChanged().AddLambda(
            [=, this, &bDidFire](
                UE::Online::FAccountId EvProductUserId,
                const TSharedPtr<const FInternetAddr> &EvInternetAddr,
                const TArray<TSharedRef<const FInternetAddr>> &EvDeveloperInternetAddrs) {
                this->TestTrue("EvInternetAddr was valid", EvInternetAddr.IsValid());
                bDidFire = true;
            });
        ListenTracker->RemoveListeningState(UserId, RegAddr1);
        TestFalse("OnListeningStateChanged() did fire", bDidFire);
        ListenTracker->OnListeningStateChanged().Remove(Delegate);
    }

    // Get returns true after deregistration (SKT1 #1) and returns (SKT1).
    {
        TSharedPtr<const FInternetAddr> InternetAddr;
        TArray<TSharedRef<const FInternetAddr>> DeveloperAddrs;
        ListenTracker->GetListeningState(UserId, InternetAddr, DeveloperAddrs);
        if (TestTrue("Found address for user", InternetAddr.IsValid()))
        {
            TestTrue("Internet addrs equal Get", *InternetAddr == *RegAddr1);
        }
    }

    // OnListeningStateChanged() fires when RemoveListeningStateing (SKT1 #2), and returns SKT2.
    {
        bool bDidFire = false;
        FDelegateHandle Delegate = ListenTracker->OnListeningStateChanged().AddLambda(
            [this, UserId, RegAddr2, &bDidFire](
                UE::Online::FAccountId EvProductUserId,
                const TSharedPtr<const FInternetAddr> &EvInternetAddr,
                const TArray<TSharedRef<const FInternetAddr>> &EvDeveloperInternetAddrs) {
                if (this->TestTrue("EvInternetAddr was valid", EvInternetAddr.IsValid()))
                {
                    this->TestEqual("Product user IDs equal OnListeningStateChanged()", UserId, EvProductUserId);
                    this->TestTrue("Internet addrs equal OnListeningStateChanged()", *EvInternetAddr == *RegAddr2);
                }
                bDidFire = true;
            });
        ListenTracker->RemoveListeningState(UserId, RegAddr1);
        TestTrue("OnListeningStateChanged() did fire", bDidFire);
        ListenTracker->OnListeningStateChanged().Remove(Delegate);
    }

    // Get returns true after deregistration (SKT1 #2) and returns (SKT2).
    {
        TSharedPtr<const FInternetAddr> InternetAddr;
        TArray<TSharedRef<const FInternetAddr>> DeveloperAddrs;
        ListenTracker->GetListeningState(UserId, InternetAddr, DeveloperAddrs);
        if (TestTrue("Found address for user", InternetAddr.IsValid()))
        {
            TestTrue("Internet addrs equal Get", *InternetAddr == *RegAddr2);
        }
    }

    // OnClosed fires when RemoveListeningStateing (SKT2 #1).
    {
        bool bDidFire = false;
        FDelegateHandle Delegate = ListenTracker->OnListeningStateChanged().AddLambda(
            [this, UserId, &bDidFire](
                UE::Online::FAccountId EvProductUserId,
                const TSharedPtr<const FInternetAddr> &EvInternetAddr,
                const TArray<TSharedRef<const FInternetAddr>> &EvDeveloperInternetAddrs) {
                this->TestFalse("EvInternetAddr was not valid", EvInternetAddr.IsValid());
                this->TestEqual("Product user IDs equal OnListeningStateChanged()", UserId, EvProductUserId);
                bDidFire = true;
            });
        ListenTracker->RemoveListeningState(UserId, RegAddr2);
        TestTrue("OnListeningStateChanged() did fire", bDidFire);
        ListenTracker->OnListeningStateChanged().Remove(Delegate);
    }

    return true;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()