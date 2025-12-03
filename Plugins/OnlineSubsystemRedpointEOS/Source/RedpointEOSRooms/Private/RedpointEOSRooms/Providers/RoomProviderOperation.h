// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "Dom/JsonObject.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1191434334, Redpoint::EOS::Rooms::Providers)
{

class IRoomProviderOperation
{
#if !NO_LOGGING
protected:
    virtual TSharedRef<FJsonObject> &GetMutableDebugJson() = 0;
#endif

public:
    IRoomProviderOperation() = default;
    UE_NONCOPYABLE(IRoomProviderOperation);
    virtual ~IRoomProviderOperation() = default;

#if !NO_LOGGING
    /**
     * Initializes the debug JSON state (usually by performing all read-related operations).
     */
    virtual void InitializeDebugJson() = 0;

    /**
     * Returns the current debug JSON state.
     */
    [[nodiscard]] TSharedRef<const FJsonObject> GetDebugJson();
#endif
};

}

namespace Redpoint::EOS::Rooms::Providers
{
REDPOINT_EOS_FILE_NS_EXPORT(1191434334, Redpoint::EOS::Rooms::Providers, IRoomProviderOperation)
}

REDPOINT_EOS_CODE_GUARD_END()