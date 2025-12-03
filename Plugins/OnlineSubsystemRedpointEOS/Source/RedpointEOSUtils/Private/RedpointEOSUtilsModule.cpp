// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSUtilsModule.h"

#include "OnlineEngineInterfaceEOS.h"
#include "UObject/Package.h"

void FRedpointEOSUtilsModule::StartupModule()
{
// @note: As of Unreal Engine 5.5, a configuration option is now available to change which OnlineEngineInterfaceImpl
// gets used, so we do that instead.
#if !REDPOINT_EOS_UE_5_5_OR_LATER
    if (IsRunningCommandlet())
    {
        // If this hack is active when IoStore packaging runs, it causes a crash. Turn off this hack if running as a
        // commandlet (since EOS will also be turned off as well, as per the check in
        // OnlineSubsystemRedpointEOSModule.cpp).
        return;
    }

    UClass *OnlineEngineInterfaceClass = StaticLoadClass(
        UOnlineEngineInterfaceEOS::StaticClass(),
        NULL,
        TEXT("/Script/RedpointEOSUtils.OnlineEngineInterfaceEOS"),
        NULL,
        LOAD_Quiet,
        NULL);
    if (OnlineEngineInterfaceClass)
    {
        UOnlineEngineInterfaceEOS *Obj =
            NewObject<UOnlineEngineInterfaceEOS>(GetTransientPackage(), OnlineEngineInterfaceClass);
        Obj->DoHack();
    }
#endif
}

IMPLEMENT_MODULE(FRedpointEOSUtilsModule, RedpointEOSUtils)