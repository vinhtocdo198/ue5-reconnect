// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Configurators/AuthenticationConfigurator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3054014618, Redpoint::EOS::Editor::Config::Configurators)
{

#define READ_WIDGET_CLASS(Class)                                                                                       \
    if (Reader.GetString(TEXT("WidgetClass_") TEXT(#Class), Value))                                                    \
    {                                                                                                                  \
        Instance->WidgetClass_##Class = FSoftClassPath(*Value);                                                        \
    }
#define WRITE_WIDGET_CLASS(Class)                                                                                      \
    Writer.SetString(TEXT("WidgetClass_") TEXT(#Class), *Instance->WidgetClass_##Class.ToString());

void FAuthenticationConfigurator::InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    Instance->DevAuthToolAddress = TEXT("localhost:6300");
    Instance->DevAuthToolDefaultCredentialName = TEXT("Context_1");
    Instance->AuthenticationGraph = NAME_Default;
    Instance->EditorAuthenticationGraph = NAME_Default;
    Instance->CrossPlatformAccountProvider = NAME_None;
    Instance->RequireCrossPlatformAccount = false;
    Instance->SimpleFirstPartyLoginUrl = TEXT("");

    // NOTE: These defaults need to be kept in sync with the authentication graph.
    Instance->WidgetClass_EnterDevicePinCode =
        FSoftClassPath(FString(TEXT("/Script/RedpointEOSAuth.EOSDefaultUserInterface_EnterDevicePinCode")));
    Instance->WidgetClass_SignInOrCreateAccount =
        FSoftClassPath(FString(TEXT("/Script/RedpointEOSAuth.EOSDefaultUserInterface_SignInOrCreateAccount")));
}

void FAuthenticationConfigurator::Load(
    FConfiguratorContext &Context,
    IConfigurationReader &Reader,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    FString Value;
    if (Reader.GetString(TEXT("AuthenticationGraph"), Value))
    {
        Instance->AuthenticationGraph = FName(*Value);
    }
    else if (Reader.GetString(TEXT("AuthenticationBehaviour"), Value))
    {
        // We are migrating this project from the old AuthenticationBehaviour value to the new settings.
        if (Value == "NoEAS")
        {
            Instance->CrossPlatformAccountProvider = NAME_None;
            Instance->RequireCrossPlatformAccount = false;
        }
        else if (Value == "EASOptional")
        {
            Instance->CrossPlatformAccountProvider = FName(TEXT("EpicGames"));
            Instance->RequireCrossPlatformAccount = false;
        }
        else if (Value == "EASRequired")
        {
            Instance->CrossPlatformAccountProvider = FName(TEXT("EpicGames"));
            Instance->RequireCrossPlatformAccount = true;
        }
    }
    if (Reader.GetString(TEXT("EditorAuthenticationGraph"), Value))
    {
        Instance->EditorAuthenticationGraph = FName(*Value);
    }
    if (Reader.GetString(TEXT("CrossPlatformAccountProvider"), Value))
    {
        Instance->CrossPlatformAccountProvider = FName(*Value);
    }
    Reader.GetBool(TEXT("RequireCrossPlatformAccount"), Instance->RequireCrossPlatformAccount);
    Reader.GetString(TEXT("DevAuthToolAddress"), Instance->DevAuthToolAddress);
    Reader.GetString(TEXT("DevAuthToolDefaultCredentialName"), Instance->DevAuthToolDefaultCredentialName);
    Reader.GetString(TEXT("SimpleFirstPartyLoginUrl"), Instance->SimpleFirstPartyLoginUrl);

    READ_WIDGET_CLASS(EnterDevicePinCode);
    READ_WIDGET_CLASS(SignInOrCreateAccount);
}

void FAuthenticationConfigurator::Save(
    FConfiguratorContext &Context,
    IConfigurationWriter &Writer,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    Writer.SetString(TEXT("AuthenticationGraph"), *Instance->AuthenticationGraph.ToString());
    Writer.SetString(TEXT("EditorAuthenticationGraph"), *Instance->EditorAuthenticationGraph.ToString());
    Writer.SetString(TEXT("CrossPlatformAccountProvider"), *Instance->CrossPlatformAccountProvider.ToString());
    Writer.SetBool(TEXT("RequireCrossPlatformAccount"), Instance->RequireCrossPlatformAccount);
    Writer.SetString(TEXT("DevAuthToolAddress"), *Instance->DevAuthToolAddress);
    Writer.SetString(TEXT("DevAuthToolDefaultCredentialName"), *Instance->DevAuthToolDefaultCredentialName);
    Writer.SetString(TEXT("SimpleFirstPartyLoginUrl"), *Instance->SimpleFirstPartyLoginUrl);

    WRITE_WIDGET_CLASS(EnterDevicePinCode);
    WRITE_WIDGET_CLASS(SignInOrCreateAccount);
}

}

REDPOINT_EOS_CODE_GUARD_END()