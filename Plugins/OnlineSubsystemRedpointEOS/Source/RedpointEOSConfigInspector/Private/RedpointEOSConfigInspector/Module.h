// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "Containers/ObservableArray.h"
#include "Widgets/Docking/SDockTab.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

class FModule : public IModuleInterface
{

#if REDPOINT_EOS_UE_5_5_OR_LATER

public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    TWeakPtr<SDockTab> ConfigInspectorTab;
    TSharedRef<SDockTab> CreateConfigInspectorTab(const FSpawnTabArgs &Args);
    TSharedPtr<SWidget> CreateConfigInspectorDialog();

    static void PopulateObservableArray(
        const TSharedRef<UE::Slate::Containers::TObservableArray<TSharedPtr<class FConfigEntry>>>
            &ConfigObservableArray,
        FName PlatformName);

#endif
};

REDPOINT_EOS_CODE_GUARD_END()