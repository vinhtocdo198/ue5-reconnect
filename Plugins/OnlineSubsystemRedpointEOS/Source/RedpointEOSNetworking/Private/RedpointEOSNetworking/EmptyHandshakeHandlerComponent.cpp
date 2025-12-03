// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/EmptyHandshakeHandlerComponent.h"
#include "Containers/Ticker.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1870867386, Redpoint::EOS::Networking)
{

FEmptyHandshakeHandlerComponent::FEmptyHandshakeHandlerComponent()
{
    bRequiresHandshake = true;
}

void FEmptyHandshakeHandlerComponent::NotifyHandshakeBegin()
{
    SetActive(true);
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSPLambda(
            this,
            [this](float DeltaSeconds) {
                SetState(UE::Handler::Component::State::Initialized);
                Initialized();
                return false;
            }),
        0.0f);
}

bool FEmptyHandshakeHandlerComponent::IsValid() const
{
    return true;
}

int32 FEmptyHandshakeHandlerComponent::GetReservedPacketBits() const
{
    return 0;
}

void FEmptyHandshakeHandlerComponent::Initialize()
{
}

void FEmptyHandshakeHandlerComponent::SetAsInitialized()
{
    SetState(UE::Handler::Component::State::Initialized);
    Initialized();
}

}

REDPOINT_EOS_CODE_GUARD_END()
