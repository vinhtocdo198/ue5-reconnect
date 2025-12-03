// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(534086287, Redpoint::EOS::Framework::Extra)
{

class REDPOINTEOSFRAMEWORKEXTRA_API FRedpointEOSFrameworkExtraModule : public FDefaultModuleImpl
{
};

}

namespace Redpoint::EOS::Framework::Extra
{
REDPOINT_EOS_FILE_NS_EXPORT(534086287, Redpoint::EOS::Framework::Extra, FRedpointEOSFrameworkExtraModule)
}

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(::Redpoint::EOS::Framework::Extra::FRedpointEOSFrameworkExtraModule, RedpointEOSFrameworkExtra);