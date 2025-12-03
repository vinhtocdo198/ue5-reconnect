// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2330899593, Redpoint::EOS::Editor::Config)
{

enum EConfigurationFileType
{
    /**
     * DefaultEngine.ini or a platform's Engine.ini file.
     */
    Engine,

    /**
     * DefaultGame.ini
     */
    Game,

    /**
     * DefaultEditor.ini
     */
    Editor,

    /**
     * DedicatedServerEngine.ini
     */
    DedicatedServer,

    /**
     * Build/NoRedist/TrustedEOSClient.ini
     */
    TrustedClient,
};

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(2330899593, Redpoint::EOS::Editor::Config, EConfigurationFileType)
}

REDPOINT_EOS_CODE_GUARD_END()
