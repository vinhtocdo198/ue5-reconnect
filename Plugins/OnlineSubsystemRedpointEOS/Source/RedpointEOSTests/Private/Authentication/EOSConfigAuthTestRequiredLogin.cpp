// Copyright June Rhodes. All Rights Reserved.

#if 0 // @todo Restore these tests

#include "RedpointEOSTests/Private/Authentication/EOSConfigAuthTestRequiredLogin.h"

FName FEOSConfigAuthTestRequiredLogin::GetAuthenticationGraph() const
{
    return FName(TEXT("AutomatedTestingOSS"));
}

FName FEOSConfigAuthTestRequiredLogin::GetCrossPlatformAccountProvider() const
{
    return FName(TEXT("AutomatedTesting"));
}

bool FEOSConfigAuthTestRequiredLogin::GetRequireCrossPlatformAccount() const
{
    return true;
}

bool FEOSConfigAuthTestRequiredLogin::IsAutomatedTesting() const
{
    return true;
}

#endif