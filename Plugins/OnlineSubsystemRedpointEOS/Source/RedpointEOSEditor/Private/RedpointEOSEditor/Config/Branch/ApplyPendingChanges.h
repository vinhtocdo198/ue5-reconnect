// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSEditor/Config/Branch/ConfigIndex.h"
#include "RedpointEOSEditor/Config/ConfigurationWriterPendingChanges.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2590437723, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Editor::Config;
using namespace ::Redpoint::EOS::Editor::Config::Branch;

FString ApplyPendingChanges(FConfigIndex &ConfigIndex, const FConfigurationWriterPendingChanges &PendingChanges);

}

namespace Redpoint::EOS::Editor::Config::Branch
{
REDPOINT_EOS_FILE_NS_EXPORT(2590437723, Redpoint::EOS::Editor::Config::Branch, ApplyPendingChanges);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
