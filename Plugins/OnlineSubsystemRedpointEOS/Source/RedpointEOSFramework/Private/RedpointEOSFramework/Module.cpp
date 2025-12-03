// Copyright June Rhodes. All Rights Reserved.

#include "LogRedpointEOSFramework.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSFramework/CreateOrJoinPartyIdentityHook.h"
#include "RedpointEOSFramework/CreatePartyOnLeaveRoomFilter.h"
#include "RedpointEOSIdentity/IdentityHookRegistry.h"
#include "RedpointEOSRooms/Filters/RoomFilterRegistry.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2627960690, Redpoint::EOS::Framework)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
using namespace ::Redpoint::EOS::Framework;
using namespace ::Redpoint::EOS::Rooms::Filters;
using namespace ::Redpoint::EOS::Identity;
#endif

class REDPOINTEOSFRAMEWORK_API FRedpointEOSFrameworkModule : public FDefaultModuleImpl
{
private:
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    TSharedPtr<FCreateOrJoinPartyIdentityHook> IdentityHook;
    TSharedPtr<FCreatePartyOnLeaveRoomFilter> RoomFilter;
#endif

public:
    virtual void StartupModule() override
    {
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
        this->IdentityHook = MakeShared<FCreateOrJoinPartyIdentityHook>();
        FIdentityHookRegistry::Register(this->IdentityHook.ToSharedRef(), 20000);

        this->RoomFilter = MakeShared<FCreatePartyOnLeaveRoomFilter>();
        FRoomFilterRegistry::Register(this->RoomFilter.ToSharedRef(), 10000);
#endif

        bool bEnableFrameworkExtraModule = false;
        GConfig->GetBool(
            TEXT("EpicOnlineServices"),
            TEXT("EnableFrameworkExtraModule"),
            bEnableFrameworkExtraModule,
            GEngineIni);
        if (bEnableFrameworkExtraModule)
        {
            UE_LOG(
                LogRedpointEOSFramework,
                Verbose,
                TEXT("Loading RedpointEOSFrameworkExtra module because the 'EnableFrameworkExtraModule' configuration "
                     "option has been turned on."));
            FModuleManager::Get().LoadModuleChecked(FName(TEXT("RedpointEOSFrameworkExtra")));
        }
    }

    virtual void ShutdownModule() override
    {
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
        if (this->IdentityHook.IsValid())
        {
            FIdentityHookRegistry::Unregister(this->IdentityHook.ToSharedRef());
            this->IdentityHook.Reset();
        }

        if (this->RoomFilter.IsValid())
        {
            FRoomFilterRegistry::Unregister(this->RoomFilter.ToSharedRef());
            this->RoomFilter.Reset();
        }
#endif
    }
};

}

namespace Redpoint::EOS::Framework
{
REDPOINT_EOS_FILE_NS_EXPORT(2627960690, Redpoint::EOS::Framework, FRedpointEOSFrameworkModule)
}

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(::Redpoint::EOS::Framework::FRedpointEOSFrameworkModule, RedpointEOSFramework);