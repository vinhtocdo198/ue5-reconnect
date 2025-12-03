// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Common
{

const TSet<EOnlinePlatformType> OnlinePlatformTypes_AllowedPlatforms_All = TSet<EOnlinePlatformType>{
    EOnlinePlatformType::Unknown,
    EOnlinePlatformType::PSN,
    EOnlinePlatformType::Switch,
    EOnlinePlatformType::XBL,
};

const TSet<EOnlinePlatformType> OnlinePlatformTypes_AllowedPlatforms_Console = TSet<EOnlinePlatformType>{
    EOnlinePlatformType::PSN,
    EOnlinePlatformType::Switch,
    EOnlinePlatformType::XBL,
};

FString OnlinePlatformTypeToString(EOnlinePlatformType InType)
{
    switch (InType)
    {
    case EOnlinePlatformType::Unknown:
        return TEXT("Unknown");
    case EOnlinePlatformType::Epic:
        return TEXT("Epic");
    case EOnlinePlatformType::PSN:
        return TEXT("PSN");
    case EOnlinePlatformType::Switch:
        return TEXT("Switch");
    case EOnlinePlatformType::XBL:
        return TEXT("XBL");
    case EOnlinePlatformType::Steam:
        return TEXT("Steam");
    default:
        return FString::Printf(TEXT("Unknown (%u)"), (uint32)InType);
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()