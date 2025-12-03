// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Configurators/StorageConfigurator.h"

#include "RedpointEOSEditorModule.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(428911753, Redpoint::EOS::Editor::Config::Configurators)
{

void FStorageConfigurator::InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
}

void FStorageConfigurator::Load(
    FConfiguratorContext &Context,
    IConfigurationReader &Reader,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Reader.GetString(TEXT("PlayerDataEncryptionKey"), Instance->PlayerDataEncryptionKey);
}

bool FStorageConfigurator::Validate(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    TArray<TCHAR> PermittedChars = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    bool bGenerate = false;
    if (Instance->PlayerDataEncryptionKey.IsEmpty() || Instance->PlayerDataEncryptionKey.Len() != 64)
    {
        bGenerate = true;
    }
    else
    {
        for (const auto &Char : Instance->PlayerDataEncryptionKey)
        {
            if (!PermittedChars.Contains(Char))
            {
                bGenerate = true;
                break;
            }
        }
    }

    if (bGenerate)
    {
        Instance->PlayerDataEncryptionKey = TEXT("");
        for (int32 i = 0; i < 64; i++)
        {
            Instance->PlayerDataEncryptionKey += PermittedChars[FMath::Rand() % PermittedChars.Num()];
        }
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Detect missing or incorrect data encryption key. It has been automatically generated and your "
                 "configuration files will be be automatically updated."));
        return true;
    }

    return false;
}

void FStorageConfigurator::Save(
    FConfiguratorContext &Context,
    IConfigurationWriter &Writer,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Writer.SetString(TEXT("PlayerDataEncryptionKey"), Instance->PlayerDataEncryptionKey);
}

}

REDPOINT_EOS_CODE_GUARD_END()