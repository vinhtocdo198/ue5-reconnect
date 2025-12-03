// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1264771280, Redpoint::EOS::Editor::Config)
{

enum EConfigurationSettingType
{
    Bool,

    Int32,

    String,

    StringArray,
};

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(1264771280, Redpoint::EOS::Editor::Config, EConfigurationSettingType)
}

REDPOINT_EOS_CODE_GUARD_END()
