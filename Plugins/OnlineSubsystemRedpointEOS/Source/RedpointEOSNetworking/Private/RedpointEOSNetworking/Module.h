// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/NetDriver.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(731365916, Redpoint::EOS::Networking)
{

class FRedpointEOSNetworkingModule : public FDefaultModuleImpl
{
private:
    FDelegateHandle OnNetDriverCreatedHandle;
    FDelegateHandle OnBeforeAnyTickHandle;

    void OnNetDriverCreated(UWorld *World, UNetDriver *NetDriver);

public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

}

namespace Redpoint::EOS::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(731365916, Redpoint::EOS::Networking, FRedpointEOSNetworkingModule)
}

REDPOINT_EOS_CODE_GUARD_END()