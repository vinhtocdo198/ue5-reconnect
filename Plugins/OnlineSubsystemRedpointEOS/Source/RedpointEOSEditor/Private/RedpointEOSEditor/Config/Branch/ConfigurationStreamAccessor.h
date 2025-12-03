// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "Misc/ConfigCacheIni.h"
#include "Misc/ConfigContext.h"
#include "RedpointEOSEditor/Config/ConfigurationWriter.h"
#include "RedpointEOSEditor/Config/ConfigurationWriterPendingChanges.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(564516167, Redpoint::EOS::Editor::Config::Branch)
{
using namespace ::Redpoint::EOS::Editor::Config;

class FConfigurationStreamAccessor
{
private:
    struct FInstantiatedConfigFile
    {
        FString FilePath;
        TSharedRef<FConfigCacheIni> ConfigSystem;
        FConfigFile File;
        FConfigContext Context;
        FConfigBranch *Branch;
        FInstantiatedConfigFile(const FString &InFilePath);
        void ReloadIfNotLoaded();
    };
    static TMap<TTuple<EConfigurationFileType, FName>, TSharedPtr<FInstantiatedConfigFile>> InstantiatedConfigFiles;

    static TSharedRef<FInstantiatedConfigFile> GetInstantiatedFile(EConfigurationFileType File, FName Platform);

public:
    static FConfigBranch *GetConfigBranchUnchecked(EConfigurationFileType File, FName Platform, FString &FilePath);

    /**
     * Finds a static layer in a branch for the target file path, taking into account scenarios where the file path in
     * the static layer map might be a different string but point to the desired file (e.g. the is an absolute path and
     * the static layer map is storing the relative path).
     */
    static const FConfigCommandStream *FindStaticLayer(FConfigBranch *Branch, const FString &FilePath);
};

}

namespace Redpoint::EOS::Editor::Config::Branch
{
REDPOINT_EOS_FILE_NS_EXPORT(564516167, Redpoint::EOS::Editor::Config::Branch, FConfigurationStreamAccessor);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()
