// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/ConfiguratorContext.h"
#include "OnlineSubsystemEOSEditorPrefs.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3875522204, Redpoint::EOS::Editor::Config)
{

FConfiguratorContext::FConfiguratorContext()
    : bAutomaticallyConfigureEngineLevelSettings(false)
{
    const UOnlineSubsystemEOSEditorPrefs *CDO = GetDefault<UOnlineSubsystemEOSEditorPrefs>();
    this->bAutomaticallyConfigureEngineLevelSettings = CDO->bAutomaticallyConfigureProjectForEOS;
}

}

REDPOINT_EOS_CODE_GUARD_END()
