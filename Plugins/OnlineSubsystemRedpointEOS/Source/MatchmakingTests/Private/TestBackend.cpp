// Copyright June Rhodes. All Rights Reserved.

#include "TestBackend.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

const uint8 *FOnlineLobbyIdTest::GetBytes() const
{
    return reinterpret_cast<const uint8 *>(&this->Id);
}

int32 FOnlineLobbyIdTest::GetSize() const
{
    return sizeof(this->Id);
}

bool FOnlineLobbyIdTest::IsValid() const
{
    return true;
}

FString FOnlineLobbyIdTest::ToString() const
{
    return FString::Printf(TEXT("%llu"), this->Id);
}

FString FOnlineLobbyIdTest::ToDebugString() const
{
    return FString::Printf(TEXT("%llu"), this->Id);
}

TSharedPtr<FTestBackendLobby> FTestBackend::CreateLobby()
{
    TSharedRef<FTestBackendLobby> NewLobby = MakeShared<FTestBackendLobby>();
    NewLobby->LobbyId = this->NextLobbyId++;
    return NewLobby;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()