// Copyright June Rhodes. All Rights Reserved.

#include "RedpointFriendListSection.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

bool URedpointFriendListSection::HasAnyEntries() const
{
    return this->Entries.Num() > 0;
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()