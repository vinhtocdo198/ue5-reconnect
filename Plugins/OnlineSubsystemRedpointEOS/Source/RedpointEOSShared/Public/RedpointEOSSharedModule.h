// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSAPI/SDK.h"
#endif

#if REDPOINT_EOS_UE_5_5_OR_LATER
REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(647807917, Redpoint::EOS::Shared, FRedpointEOSSDKManager)
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

class REDPOINTEOSSHARED_API FRedpointEOSSharedModule : public FDefaultModuleImpl
{
#if REDPOINT_EOS_UE_5_5_OR_LATER
private:
    TSharedPtr<::Redpoint::EOS::Shared::FRedpointEOSSDKManager> Manager;

public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    TDelegate<EOS_HPlatform(EOS_Platform_Options &PlatformOptions)> OnPlatformCreateRequested;
#endif
};

REDPOINT_EOS_CODE_GUARD_END()