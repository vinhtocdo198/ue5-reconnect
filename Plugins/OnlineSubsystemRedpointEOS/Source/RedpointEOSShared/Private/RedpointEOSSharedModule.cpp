// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSSharedModule.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSShared/IEOSSDKManager.h"
#include "RedpointEOSShared/RedpointEOSSDKManager.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

void FRedpointEOSSharedModule::StartupModule()
{
    this->Manager = MakeShared<::Redpoint::EOS::Shared::FRedpointEOSSDKManager>();

    IModularFeatures::Get().RegisterModularFeature(IEOSSDKManager::GetModularFeatureName(), this->Manager.Get());
}

void FRedpointEOSSharedModule::ShutdownModule()
{
    IModularFeatures::Get().UnregisterModularFeature(IEOSSDKManager::GetModularFeatureName(), this->Manager.Get());

    this->Manager.Reset();
}

#endif

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(FRedpointEOSSharedModule, RedpointEOSShared);