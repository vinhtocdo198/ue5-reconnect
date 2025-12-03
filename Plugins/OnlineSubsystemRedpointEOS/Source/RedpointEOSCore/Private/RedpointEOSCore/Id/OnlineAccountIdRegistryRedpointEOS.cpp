// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Id/OnlineAccountIdRegistryRedpointEOS.h"

#include "RedpointEOSCore/Id/OnlineAccountIdRegistryRedpointEOSImpl.h"
#include "RedpointEOSCore/Logging.h"

namespace REDPOINT_EOS_FILE_NS_ID(2175684931, Redpoint::EOS::Core::Id)
{

IOnlineAccountIdRegistryRedpointEOS &IOnlineAccountIdRegistryRedpointEOS::GetRegistered()
{
    IOnlineAccountIdRegistry *Registry = FOnlineIdRegistryRegistry::Get().GetAccountIdRegistry(EOnlineServices::Epic);
    checkf(Registry, TEXT("Expected Epic online ID registry to be registered."));
    return *static_cast<IOnlineAccountIdRegistryRedpointEOS *>(Registry);
}

}