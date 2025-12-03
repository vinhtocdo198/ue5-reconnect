// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAsync/Task.h"
#include "RedpointEOSCore/Utils/NamedTuple.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_ITCH_IO_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(2060926592, Redpoint::EOS::Platform::Integration::ItchIo)
{
using namespace ::Redpoint::EOS::Async;
using namespace ::Redpoint::EOS::API;

/**
 * Represents the locally signed in itch.io user.
 */
REDPOINT_AGGREGATE(FItchIoLocalUser, int32, Id, FString, Username, FString, DisplayName, FString, CoverUrl);

/**
 * Tracks the currently signed in itch.io user and their metadata, since there's no online subsystem implementation for
 * this platform.
 */
class FItchIoSystem : public IPlatformInstanceSystem, public TSharedFromThis<FItchIoSystem>
{
public:
    static FName GetSystemName();
    static TSharedRef<FItchIoSystem> ConstructSystem(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment);

private:
    TOptional<FItchIoLocalUser> CurrentLocalUser;

public:
    FItchIoSystem() = default;
    UE_NONCOPYABLE(FItchIoSystem);
    virtual ~FItchIoSystem() override = default;

    [[nodiscard]] TTask<FError> TryLoginLocalUserFromApiKey();

    TOptional<FItchIoLocalUser> GetLocalUser();
};

}

namespace Redpoint::EOS::Platform::Integration::ItchIo
{
REDPOINT_EOS_FILE_NS_EXPORT(2060926592, Redpoint::EOS::Platform::Integration::ItchIo, FItchIoLocalUser)
REDPOINT_EOS_FILE_NS_EXPORT(2060926592, Redpoint::EOS::Platform::Integration::ItchIo, FItchIoSystem)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()