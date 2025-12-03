// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class REDPOINTEOSAUTH_API FRedpointEOSAuthModule : public FDefaultModuleImpl
{
};

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(FRedpointEOSAuthModule, RedpointEOSAuth);
