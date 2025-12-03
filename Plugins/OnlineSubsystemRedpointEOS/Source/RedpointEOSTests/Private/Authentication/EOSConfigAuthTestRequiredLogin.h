// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if 0 // @todo Restore these tests

#include "RedpointEOSTests/Public/TestHelpers.h"

class REDPOINTEOSTESTS_API FEOSConfigAuthTestRequiredLogin : public FEOSConfigEASLogin
{
public:
    FEOSConfigAuthTestRequiredLogin(){};
    UE_NONCOPYABLE(FEOSConfigAuthTestRequiredLogin);
    virtual ~FEOSConfigAuthTestRequiredLogin(){};

    virtual FName GetAuthenticationGraph() const override;
    virtual FName GetCrossPlatformAccountProvider() const override;
    virtual bool GetRequireCrossPlatformAccount() const override;
    virtual bool IsAutomatedTesting() const override;
};

#endif