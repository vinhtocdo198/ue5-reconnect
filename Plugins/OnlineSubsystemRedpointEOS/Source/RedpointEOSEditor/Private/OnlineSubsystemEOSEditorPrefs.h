// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "OnlineSubsystemEOSEditorPrefs.generated.h"

/**
 * Editor preferences for Epic Online Services.
 */
UCLASS(Config = EditorPerProjectUserSettings, meta = (DisplayName = "Epic Online Services"))
class UOnlineSubsystemEOSEditorPrefs : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UOnlineSubsystemEOSEditorPrefs(const FObjectInitializer &ObjectInitializer);

    /**
     * If turned on, the EOS dropdown will not be visible in the editor toolbar.
     */
    UPROPERTY(EditAnywhere, Config, Category = "Toolbar")
    bool bHideDropdownInEditorToolbar;

    /**
     * If turned on, the EOS plugin will automatically configure the project to use EOS. This changes the default
     * subsystem for the project, it's networking drivers, and various other engine configuration settings that must be
     * set in order for EOS to work.
     *
     * If you'd rather set these settings manually, you can turn this option off.
     */
    UPROPERTY(EditAnywhere, Config, Category = "Automation")
    bool bAutomaticallyConfigureProjectForEOS;

    /**
     * If turned on, the blueprint nodes that offer bindings to the online subsystem (OSSv1) APIs will be hidden and no
     * longer appear in the blueprint editor auto-complete. You can turn this option on if you would like to exclusively
     * work with the online subsystem APIs from C++, and want to prevent accidental usage by other team members.
     */
    UPROPERTY(
        EditAnywhere,
        Config,
        Category = "Development",
        meta = (DisplayName = "Hide Blueprint Nodes for accessing Online Subsystems (OSSv1)"))
    bool HideBlueprintsForOSSv1FromEditor;
};