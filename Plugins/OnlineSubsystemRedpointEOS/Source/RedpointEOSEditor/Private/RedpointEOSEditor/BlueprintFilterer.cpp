// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/BlueprintFilterer.h"

#include "AutoSubsystems/OnlineAchievementsSubsystem.h"
#include "AutoSubsystems/OnlineAvatarSubsystem.h"
#include "AutoSubsystems/OnlineChatSubsystem.h"
#include "AutoSubsystems/OnlineEntitlementsSubsystem.h"
#include "AutoSubsystems/OnlineEventsSubsystem.h"
#include "AutoSubsystems/OnlineExternalUISubsystem.h"
#include "AutoSubsystems/OnlineFriendsSubsystem.h"
#include "AutoSubsystems/OnlineGameActivitySubsystem.h"
#include "AutoSubsystems/OnlineGameItemStatsSubsystem.h"
#include "AutoSubsystems/OnlineGroupsSubsystem.h"
#include "AutoSubsystems/OnlineIdentitySubsystem.h"
#include "AutoSubsystems/OnlineLeaderboardsSubsystem.h"
#include "AutoSubsystems/OnlineLobbySubsystem.h"
#include "AutoSubsystems/OnlineMessageSanitizerSubsystem.h"
#include "AutoSubsystems/OnlineMessageSubsystem.h"
#include "AutoSubsystems/OnlinePartySubsystem.h"
#include "AutoSubsystems/OnlinePresenceSubsystem.h"
#include "AutoSubsystems/OnlinePurchaseSubsystem.h"
#include "AutoSubsystems/OnlineSessionSubsystem.h"
#include "AutoSubsystems/OnlineSharedCloudSubsystem.h"
#include "AutoSubsystems/OnlineSharingSubsystem.h"
#include "AutoSubsystems/OnlineStatsSubsystem.h"
#include "AutoSubsystems/OnlineStoreV2Subsystem.h"
#include "AutoSubsystems/OnlineSubsystemBP.h"
#include "AutoSubsystems/OnlineTimeSubsystem.h"
#include "AutoSubsystems/OnlineTitleFileSubsystem.h"
#include "AutoSubsystems/OnlineTournamentSubsystem.h"
#include "AutoSubsystems/OnlineTurnBasedSubsystem.h"
#include "AutoSubsystems/OnlineUserCloudSubsystem.h"
#include "AutoSubsystems/OnlineUserSubsystem.h"
#include "AutoSubsystems/OnlineVoiceAdminSubsystem.h"
#include "AutoSubsystems/OnlineVoiceChatSubsystem.h"
#include "AutoSubsystems/OnlineVoiceSubsystem.h"
#include "BlueprintActionDatabase.h"
#include "BlueprintEditorSettings.h"
#include "OnlineSubsystemEOSEditorPrefs.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3119426896, Redpoint::EOS::Editor)
{

void FBlueprintFilterer::RegisterEvents()
{
    auto *EditorSettings = GetMutableDefault<UBlueprintEditorSettings>();
    if (!IsValid(EditorSettings))
    {
        return;
    }

    auto IsClassAllowed = UBlueprintEditorSettings::FOnIsClassAllowed::CreateLambda([](const UClass *InClass) -> bool {
        auto *EditorPrefs = GetDefault<UOnlineSubsystemEOSEditorPrefs>();
        if (!IsValid(EditorPrefs) || !EditorPrefs->HideBlueprintsForOSSv1FromEditor || !IsValid(InClass))
        {
            return true;
        }
        return !InClass->GetClassPathName().GetPackageName().IsEqual(FName(TEXT("/Script/OnlineSubsystemBlueprints")));
    });
    auto IsClassPathAllowed = UBlueprintEditorSettings::FOnIsClassPathAllowed::CreateLambda(
        [](const FTopLevelAssetPath &InClassPath) -> bool {
            auto *EditorPrefs = GetDefault<UOnlineSubsystemEOSEditorPrefs>();
            if (!IsValid(EditorPrefs) || !EditorPrefs->HideBlueprintsForOSSv1FromEditor)
            {
                return true;
            }
            return !InClassPath.GetPackageName().IsEqual(FName(TEXT("/Script/OnlineSubsystemBlueprints")));
        });

    // #error need to set/remove this flag on the online subsystem blueprint classes, and periodically apply them,
    // causing a refresh if the flag actually needed to be changed...

    EditorSettings->RegisterIsClassAllowedDelegate(FName(TEXT("RedpointEOSEditor_Class")), IsClassAllowed);
    EditorSettings->RegisterIsClassAllowedOnPinDelegate(FName(TEXT("RedpointEOSEditor_ClassOnPin")), IsClassAllowed);
    EditorSettings->RegisterIsClassPathAllowedDelegate(FName(TEXT("RedpointEOSEditor_ClassPath")), IsClassPathAllowed);
    EditorSettings->RegisterIsClassPathAllowedOnPinDelegate(
        FName(TEXT("RedpointEOSEditor_ClassPathOnPin")),
        IsClassPathAllowed);

    auto *EditorPrefs = GetMutableDefault<UOnlineSubsystemEOSEditorPrefs>();
    if (IsValid(EditorPrefs))
    {
        this->OnSettingChangedDelegateHandle =
            EditorPrefs->OnSettingChanged().AddSP(this, &FBlueprintFilterer::OnSettingChanged);
    }

    this->SyncSubsystemNodeVisibility();
}

FBlueprintFilterer::~FBlueprintFilterer()
{
    if (IsEngineExitRequested())
    {
        // Not safe to use GetMutableDefault on shutdown, as this causes a crash in CoreUObject.
        return;
    }

    auto *EditorSettings = GetMutableDefault<UBlueprintEditorSettings>();
    if (IsValid(EditorSettings))
    {
        EditorSettings->UnregisterIsClassAllowedDelegate(FName(TEXT("RedpointEOSEditor_Class")));
        EditorSettings->UnregisterIsClassAllowedOnPinDelegate(FName(TEXT("RedpointEOSEditor_ClassOnPin")));
        EditorSettings->UnregisterIsClassPathAllowedDelegate(FName(TEXT("RedpointEOSEditor_ClassPath")));
        EditorSettings->UnregisterIsClassPathAllowedOnPinDelegate(FName(TEXT("RedpointEOSEditor_ClassPathOnPin")));
    }

    auto *EditorPrefs = GetMutableDefault<UOnlineSubsystemEOSEditorPrefs>();
    if (IsValid(EditorPrefs))
    {
        EditorPrefs->OnSettingChanged().Remove(this->OnSettingChangedDelegateHandle);
        this->OnSettingChangedDelegateHandle.Reset();
    }
}

void FBlueprintFilterer::OnSettingChanged(UObject *, FPropertyChangedEvent &Property)
{
    if (!Property.GetPropertyName().IsEqual(FName(TEXT("HideBlueprintsForOSSv1FromEditor"))))
    {
        return;
    }

    this->SyncSubsystemNodeVisibility();

    FBlueprintActionDatabase::Get().RefreshAll();
}

void FBlueprintFilterer::SyncSubsystemNodeVisibility()
{
    auto Classes = TArray<UClass *>{
        UOnlineAchievementsSubsystem::StaticClass(),  UOnlineAvatarSubsystem::StaticClass(),
        UOnlineChatSubsystem::StaticClass(),          UOnlineEntitlementsSubsystem::StaticClass(),
        UOnlineEventsSubsystem::StaticClass(),        UOnlineExternalUISubsystem::StaticClass(),
        UOnlineFriendsSubsystem::StaticClass(),       UOnlineGameActivitySubsystem::StaticClass(),
        UOnlineGameItemStatsSubsystem::StaticClass(), UOnlineGroupsSubsystem::StaticClass(),
        UOnlineIdentitySubsystem::StaticClass(),      UOnlineLeaderboardsSubsystem::StaticClass(),
        UOnlineLobbySubsystem::StaticClass(),         UOnlineMessageSanitizerSubsystem::StaticClass(),
        UOnlineMessageSubsystem::StaticClass(),       UOnlinePartySubsystem::StaticClass(),
        UOnlinePresenceSubsystem::StaticClass(),      UOnlinePurchaseSubsystem::StaticClass(),
        UOnlineSessionSubsystem::StaticClass(),       UOnlineSharedCloudSubsystem::StaticClass(),
        UOnlineSharingSubsystem::StaticClass(),       UOnlineStatsSubsystem::StaticClass(),
        UOnlineStoreV2Subsystem::StaticClass(),       UOnlineSubsystem::StaticClass(),
        UOnlineTimeSubsystem::StaticClass(),          UOnlineTitleFileSubsystem::StaticClass(),
        UOnlineTournamentSubsystem::StaticClass(),    UOnlineTurnBasedSubsystem::StaticClass(),
        UOnlineUserCloudSubsystem::StaticClass(),     UOnlineUserSubsystem::StaticClass(),
        UOnlineVoiceAdminSubsystem::StaticClass(),    UOnlineVoiceChatSubsystem::StaticClass(),
        UOnlineVoiceSubsystem::StaticClass(),
    };
    auto *EditorPrefs = GetMutableDefault<UOnlineSubsystemEOSEditorPrefs>();
    auto HideBlueprintsForOSSv1FromEditor = EditorPrefs->HideBlueprintsForOSSv1FromEditor;
    for (const auto &Class : Classes)
    {
        if (HideBlueprintsForOSSv1FromEditor)
        {
            Class->ClassFlags |= CLASS_NewerVersionExists;
        }
        else
        {
            Class->ClassFlags &= ~CLASS_NewerVersionExists;
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()