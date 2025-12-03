// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointMatchmakerConfig.h"
#if WITH_EDITOR
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

class MATCHMAKINGMATCHMAKER_API FMatchmakingMatchmakerModule : public IModuleInterface
{
private:
#if WITH_EDITOR
    TStrongObjectPtr<URedpointMatchmakerConfig> Config;
    ISettingsSectionPtr SettingsSection;
#endif

public:
    virtual void StartupModule() override
    {
#if WITH_EDITOR
        if (ISettingsModule *SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
        {
            this->Config = TStrongObjectPtr<URedpointMatchmakerConfig>(NewObject<URedpointMatchmakerConfig>());

            this->SettingsSection = SettingsModule->RegisterSettings(
                TEXT("Project"),
                TEXT("Game"),
                TEXT("Redpoint Matchmaking"),
                NSLOCTEXT("MatchmakingMatchmaker", "RedpointMatchmakerSettingsName", "Redpoint Matchmaking"),
                NSLOCTEXT(
                    "MatchmakingMatchmaker",
                    "RedpointMatchmakerSettingsDescription",
                    "Configure team-based matchmaking in your game."),
                this->Config.Get());
        }
#endif
    }

    virtual void ShutdownModule() override
    {
#if WITH_EDITOR
        if (ISettingsModule *SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
        {
            SettingsModule->UnregisterSettings("Project", "Game", "Redpoint Matchmaking");
            this->SettingsSection.Reset();
            this->Config.Reset();
        }
#endif
    }
};

REDPOINT_EOS_CODE_GUARD_END()

IMPLEMENT_MODULE(FMatchmakingMatchmakerModule, MatchmakingMatchmaker);