// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/ConfigurationEnumLookup.h"

#include "UObject/TopLevelAssetPath.h"
#include "UObject/UObjectGlobals.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(474878901, Redpoint::EOS::Editor::Config)
{

const UEnum *LookupEnum(const FString &EnumClass)
{
#if REDPOINT_EOS_UE_5_1_OR_LATER
    return FindObject<UEnum>(FTopLevelAssetPath(TEXT("/Script/RedpointEOSConfig"), *EnumClass), true);
#else
    return FindObject<UEnum>((UObject *)ANY_PACKAGE, *EnumClass, true);
#endif
}

}

REDPOINT_EOS_CODE_GUARD_END()