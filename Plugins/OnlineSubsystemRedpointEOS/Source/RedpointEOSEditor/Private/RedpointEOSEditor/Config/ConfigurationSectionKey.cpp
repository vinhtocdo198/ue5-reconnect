// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/ConfigurationSectionKey.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(269708907, Redpoint::EOS::Editor::Config)
{

FConfigurationSectionKey::FConfigurationSectionKey(const FString &InSection, const FString &InKey)
    : Section(InSection)
    , Key(InKey)
{
}

bool FConfigurationSectionKey::operator==(const FConfigurationSectionKey &Other) const
{
    return this->Section == Other.Section && this->Key == Other.Key;
}

uint32 GetTypeHash(const FConfigurationSectionKey &Val)
{
    return GetTypeHash(FString::Printf(TEXT("%s:%s"), *Val.Section, *Val.Key));
}

}

REDPOINT_EOS_CODE_GUARD_END()
