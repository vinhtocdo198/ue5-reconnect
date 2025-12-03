// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3875522204, Redpoint::EOS::Editor::Config)
{

class FConfiguratorContext
{
public:
    FConfiguratorContext();

    /**
     * Reflects the "automatically configure project for EOS" setting in the Editor Preferences.
     */
    bool bAutomaticallyConfigureEngineLevelSettings;
};

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(3875522204, Redpoint::EOS::Editor::Config, FConfiguratorContext)
}

REDPOINT_EOS_CODE_GUARD_END()
