// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAntiCheat/AntiCheatSystem.h"
#include "RedpointEOSAntiCheat/EditorAntiCheatSession.h"
#include "RedpointEOSCore/Exec/ExecWithAutoComplete.h"

/**
 * This is an emulated implementation of Anti-Cheat, which allows testing Anti-Cheat in the editor without actually
 * using the Anti-Cheat APIs (which can't work in the editor or with the debugger attached).
 */

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2381734488, Redpoint::EOS::AntiCheat)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Exec;

class FEditorAntiCheatSystemImpl : public IAntiCheatSystem,
                                   public FExecWithAutoComplete,
                                   public TSharedFromThis<FEditorAntiCheatSystemImpl>
{
private:
    /** Used for ::Exec handling. */
    TArray<TSharedRef<FEditorAntiCheatSession>> CurrentSessions;

public:
    virtual bool ShouldGameExitOnStartup() const override;

    virtual EAntiCheatImplementationType GetImplementationType() const override;

    virtual bool ShouldRemoteSkipPeerRegistration() const override;

    virtual TSharedPtr<FAntiCheatSession> CreateSession(
        bool bIsServer,
        UE::Online::FAccountId HostUserId,
        bool bIsDedicatedServerSession,
        TOptional<UE::Online::FAccountId> ListenServerUserId,
        FString ServerConnectionUrlOnClient) override;
    virtual bool DestroySession(FAntiCheatSession &Session) override;

    virtual bool RegisterPlayer(
        FAntiCheatSession &Session,
        UE::Online::FAccountId UserId,
        EOS_EAntiCheatCommonClientType ClientType,
        EOS_EAntiCheatCommonClientPlatform ClientPlatform) override;
    virtual bool UnregisterPlayer(FAntiCheatSession &Session, UE::Online::FAccountId UserId) override;

    virtual bool ReceiveNetworkMessage(
        FAntiCheatSession &Session,
        UE::Online::FAccountId SourceUserId,
        UE::Online::FAccountId TargetUserId,
        const uint8 *Data,
        uint32_t Size) override;

    virtual void RegisterAutoCompleteEntries(TArray<FAutoCompleteCommand> &) override;
    virtual bool Exec_Dev(UWorld *InWorld, const TCHAR *Cmd, FOutputDevice &Ar) override;
};

}

namespace Redpoint::EOS::AntiCheat
{
REDPOINT_EOS_FILE_NS_EXPORT(2381734488, Redpoint::EOS::AntiCheat, FEditorAntiCheatSystemImpl)
}

REDPOINT_EOS_CODE_GUARD_END()