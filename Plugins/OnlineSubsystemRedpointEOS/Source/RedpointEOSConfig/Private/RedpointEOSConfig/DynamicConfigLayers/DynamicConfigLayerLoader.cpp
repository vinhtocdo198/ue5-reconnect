// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSConfig/DynamicConfigLayers/DynamicConfigLayerLoader.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "Misc/ConfigCacheIni.h"
#include "Misc/ConfigTypes.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSConfig/DynamicConfigLayers/DynamicConfigLayerProvider.h"
#include "RedpointEOSConfig/Logging.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(2660729793, Redpoint::EOS::Config::DynamicConfigLayers)
{
using namespace ::Redpoint::EOS::Config;
using namespace ::Redpoint::EOS::Config::DynamicConfigLayers;

void FDynamicConfigLayerLoader::LoadDynamicConfigLayers()
{
    TArray<TSharedRef<IDynamicConfigLayer>> Layers = FDynamicConfigLayerProvider::GetDynamicConfigLayersToLoad();

    int32 LayerLoadCount = 0;
    for (const auto &Layer : Layers)
    {
        // Figure out what config file this layer applies to.
        FConfigBranch *Branch = nullptr;
        switch (Layer->GetLayerAppliesTo())
        {
        case EDynamicConfigLayerApplyTo::Engine:
            Branch = GConfig->FindBranch(FName("Engine"), GEngineIni);
            break;
        default:
            break;
        }
        if (!ensureAlwaysMsgf(
                Branch != nullptr,
                TEXT("Dynamic config layer '%s' applies to unknown INI file or the INI file couldn't be loaded, so the "
                     "dynamic config layer won't be loaded. This is a bug in the Redpoint EOS Online Framework plugin; "
                     "please report it to Redpoint Games along with a full log."),
                *Layer->GetLayerName()))
        {
            continue;
        }

        // Add the dynamic layer to the hierarchy.
        ensureAlwaysMsgf(
            Branch->AddDynamicLayerStringToHierarchy(
                FString::Printf(TEXT("RedpointEOSDynamic_%d_%s"), LayerLoadCount, *Branch->IniName.ToString()),
                Layer->GetLayerIniContent(),
                FName(*Layer->GetLayerName()),
                DynamicLayerPriority::Plugin,
                nullptr),
            TEXT("Dynamic config layer '%s' failed to be loaded into the config system via "
                 "AddDynamicLayerStringToHierarchy. This is a bug in the Redpoint EOS Online Framework plugin; "
                 "please report it to Redpoint Games along with a full log."),
            *Layer->GetLayerName());

        UE_LOG(
            LogRedpointEOSConfig,
            Verbose,
            TEXT("Dynamic config layer '%s' has been loaded."),
            *Layer->GetLayerName());
        LayerLoadCount++;
    }

    UE_LOG(
        LogRedpointEOSConfig,
        Verbose,
        TEXT("Dynamic config layer loader loaded %d of %d layers."),
        LayerLoadCount,
        Layers.Num());
}

static bool bDidConfigLoadHookRun = false;
void FDynamicConfigLayerLoader::RegisterEventHandlerToLoadDynamicConfigLayers()
{
    // @note: We can not load configs during EarliestPossible, because the editor hasn't loaded the project files yet
    // and thus hasn't loaded the configs for the project. If we do the load too early, then what we load into the
    // config system gets completely overwritten with the contents of DefaultEngine.ini
    //
    // We also can't use "PostConfigInit", because for plugin modules this happens after the "OnlineSubsystem" module,
    // and we need to run before that module's StartupModule happens.

    FModuleManager::Get().OnModulesChanged().AddLambda([](FName InModuleName, EModuleChangeReason InChangeReason) {
        if (InModuleName.IsEqual(FName(TEXT("OnlineSubsystem"))) &&
            InChangeReason == EModuleChangeReason::PluginDirectoryChanged)
            if (!bDidConfigLoadHookRun)
            {
                bDidConfigLoadHookRun = true;
                FDynamicConfigLayerLoader::LoadDynamicConfigLayers();
            }
    });
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()