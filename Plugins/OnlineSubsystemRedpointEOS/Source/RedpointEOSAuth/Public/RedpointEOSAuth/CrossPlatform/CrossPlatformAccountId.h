// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2919731249, Redpoint::EOS::Auth::CrossPlatform)
{

#define AUTOMATED_TESTING_ACCOUNT_ID FName(TEXT("AutomatedTesting"))
#define EPIC_GAMES_ACCOUNT_ID FName(TEXT("EpicGames"))
#define SIMPLE_FIRST_PARTY_ACCOUNT_ID FName(TEXT("SimpleFirstParty"))

/**
 * Abstraction of a cross-platform account ID
 */
class REDPOINTEOSAUTH_API FCrossPlatformAccountId : public TSharedFromThis<FCrossPlatformAccountId>
{
protected:
    FCrossPlatformAccountId() = default;

    virtual bool Compare(const FCrossPlatformAccountId &Other) const
    {
        return (GetType().IsEqual(Other.GetType()) && GetSize() == Other.GetSize()) &&
               (FMemory::Memcmp(GetBytes(), Other.GetBytes(), static_cast<SIZE_T>(GetSize())) == 0);
    }

public:
    UE_NONCOPYABLE(FCrossPlatformAccountId);
    virtual ~FCrossPlatformAccountId(){};

    /**
     *	Comparison operator
     */
    friend bool operator==(const FCrossPlatformAccountId &Lhs, const FCrossPlatformAccountId &Rhs)
    {
        return Lhs.Compare(Rhs);
    }
    friend bool operator!=(const FCrossPlatformAccountId &Lhs, const FCrossPlatformAccountId &Rhs)
    {
        return !Lhs.Compare(Rhs);
    }

    /**
     * Return the type of cross-platform account this is.
     */
    virtual FName GetType() const = 0;

    /**
     * Return the raw byte representation of this opaque data.
     */
    virtual const uint8 *GetBytes() const = 0;

    /**
     * Returns the size of the opaque data.
     */
    virtual int32 GetSize() const = 0;

    /**
     * Returns if this cross-platform account ID is valid.
     */
    virtual bool IsValid() const = 0;

    /**
     * Convert the cross-platform account ID to it's string representation.
     */
    virtual FString ToString() const = 0;
};

}

namespace Redpoint::EOS::Auth::CrossPlatform
{
REDPOINT_EOS_FILE_NS_EXPORT(2919731249, Redpoint::EOS::Auth::CrossPlatform, FCrossPlatformAccountId)
}

REDPOINT_EOS_CODE_GUARD_END()