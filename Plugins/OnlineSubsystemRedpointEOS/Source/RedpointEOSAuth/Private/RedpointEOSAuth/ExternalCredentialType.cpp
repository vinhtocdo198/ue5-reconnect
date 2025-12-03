// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/ExternalCredentialType.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1348168839, Redpoint::EOS::Auth)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

EOS_EExternalCredentialType FExternalCredentialType::FromString(const FString &InStr)
{
    if (InStr.ToUpper() == TEXT("EPIC"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_EPIC;
    }
    else if (InStr.ToUpper() == TEXT("PSN_ID_TOKEN"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_PSN_ID_TOKEN;
    }
    else if (InStr.ToUpper() == TEXT("XBL_XSTS_TOKEN"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_XBL_XSTS_TOKEN;
    }
    else if (InStr.ToUpper() == TEXT("DISCORD_ACCESS_TOKEN"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_DISCORD_ACCESS_TOKEN;
    }
    else if (InStr.ToUpper() == TEXT("GOG_SESSION_TICKET"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_GOG_SESSION_TICKET;
    }
    else if (InStr.ToUpper() == TEXT("NINTENDO_ID_TOKEN"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_NINTENDO_ID_TOKEN;
    }
    else if (InStr.ToUpper() == TEXT("NINTENDO_NSA_ID_TOKEN"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_NINTENDO_NSA_ID_TOKEN;
    }
    else if (InStr.ToUpper() == TEXT("UPLAY_ACCESS_TOKEN"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_UPLAY_ACCESS_TOKEN;
    }
    else if (InStr.ToUpper() == TEXT("OPENID_ACCESS_TOKEN"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_OPENID_ACCESS_TOKEN;
    }
    else if (InStr.ToUpper() == TEXT("DEVICEID_ACCESS_TOKEN"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN;
    }
    else if (InStr.ToUpper() == TEXT("APPLE_ID_TOKEN"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_APPLE_ID_TOKEN;
    }
    else if (InStr.ToUpper() == TEXT("GOOGLE_ID_TOKEN"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_GOOGLE_ID_TOKEN;
    }
    else if (InStr.ToUpper() == TEXT("OCULUS_USERID_NONCE"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_OCULUS_USERID_NONCE;
    }
    else if (InStr.ToUpper() == TEXT("ITCHIO_JWT"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_ITCHIO_JWT;
    }
    else if (InStr.ToUpper() == TEXT("ITCHIO_KEY"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_ITCHIO_KEY;
    }
    else if (InStr.ToUpper() == TEXT("STEAM_SESSION_TICKET"))
    {
        return EOS_EExternalCredentialType::EOS_ECT_STEAM_SESSION_TICKET;
    }
    else if (InStr.ToUpper() == TEXT("STEAM_APP_TICKET"))
    {
        checkf(false, TEXT("STEAM_APP_TICKET is no longer supported by the EOS SDK."));
        return EOS_EExternalCredentialType::EOS_ECT_STEAM_APP_TICKET;
    }

    checkf(false, TEXT("Expected known credential type for FExternalCredentialType::FromString"));
    return EOS_EExternalCredentialType::EOS_ECT_EPIC;
}

FString FExternalCredentialType::ToString(EOS_EExternalCredentialType InType)
{
    if (InType == EOS_EExternalCredentialType::EOS_ECT_EPIC)
    {
        return TEXT("EPIC");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_PSN_ID_TOKEN)
    {
        return TEXT("PSN_ID_TOKEN");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_XBL_XSTS_TOKEN)
    {
        return TEXT("XBL_XSTS_TOKEN");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_DISCORD_ACCESS_TOKEN)
    {
        return TEXT("DISCORD_ACCESS_TOKEN");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_GOG_SESSION_TICKET)
    {
        return TEXT("GOG_SESSION_TICKET");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_NINTENDO_ID_TOKEN)
    {
        return TEXT("NINTENDO_ID_TOKEN");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_NINTENDO_NSA_ID_TOKEN)
    {
        return TEXT("NINTENDO_NSA_ID_TOKEN");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_UPLAY_ACCESS_TOKEN)
    {
        return TEXT("UPLAY_ACCESS_TOKEN");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_OPENID_ACCESS_TOKEN)
    {
        return TEXT("OPENID_ACCESS_TOKEN");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN)
    {
        return TEXT("DEVICEID_ACCESS_TOKEN");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_APPLE_ID_TOKEN)
    {
        return TEXT("APPLE_ID_TOKEN");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_GOOGLE_ID_TOKEN)
    {
        return TEXT("GOOGLE_ID_TOKEN");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_OCULUS_USERID_NONCE)
    {
        return TEXT("OCULUS_USERID_NONCE");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_ITCHIO_JWT)
    {
        return TEXT("ITCHIO_JWT");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_ITCHIO_KEY)
    {
        return TEXT("ITCHIO_KEY");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_STEAM_SESSION_TICKET)
    {
        return TEXT("STEAM_SESSION_TICKET");
    }
    else if (InType == EOS_EExternalCredentialType::EOS_ECT_STEAM_APP_TICKET)
    {
        checkf(false, TEXT("STEAM_APP_TICKET is no longer supported by the EOS SDK."));
        return TEXT("STEAM_APP_TICKET");
    }

    checkf(false, TEXT("Expected known credential type for ExternalCredentialTypeToStr"));
    return TEXT("");
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()