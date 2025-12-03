// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSFriends/FriendIdentityHook.h"
#include "RedpointEOSIdentity/IdentityHookRegistry.h"
#endif

class REDPOINTEOSFRIENDS_API FRedpointEOSFriendsModule : public FDefaultModuleImpl
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
private:
    TSharedPtr<Redpoint::EOS::Identity::IIdentityHook> FriendIdentityHook;

public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Identity;
        using namespace ::Redpoint::EOS::Friends;

        this->FriendIdentityHook = MakeShared<FFriendIdentityHook>();
        FIdentityHookRegistry::Register(this->FriendIdentityHook.ToSharedRef(), 50000);
    };

    virtual void ShutdownModule() override
    {
        using namespace ::Redpoint::EOS::Identity;

        if (this->FriendIdentityHook.IsValid())
        {
            FIdentityHookRegistry::Unregister(this->FriendIdentityHook.ToSharedRef());
        }
        this->FriendIdentityHook.Reset();
    }
#endif
};

IMPLEMENT_MODULE(FRedpointEOSFriendsModule, RedpointEOSFriends);