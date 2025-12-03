// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Features/RoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(212611124, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms;

class REDPOINTEOSROOMS_API FFeatureRequestsOnRequest
{
public:
    FFeatureRequestsOnRequest() = default;

protected:
    /**
     * The features attached to the room request.
     */
    TMap<FName, TSharedPtr<Features::IRoomFeatureRequestBase>> FeatureRequests;

    FFeatureRequestsOnRequest(const FFeatureRequestsOnRequest &) = default;
    FFeatureRequestsOnRequest &operator=(const FFeatureRequestsOnRequest &) = default;

public:
    /**
     * Returns the names of the feature requests attached to this request.
     */
    TSet<FName> GetFeatureRequestNames() const;

    /**
     * Returns the base feature request if it has been requested by calling AddFeatureRequest.
     */
    TSharedPtr<Features::IRoomFeatureRequestBase> GetFeatureRequestBase(FName InFeatureName) const;

    /**
     * Request that the room has a particular feature.
     */
    template <typename TRoomFeatureRequest> void AddFeatureRequest(TSharedRef<TRoomFeatureRequest> FeatureRequest)
    {
        static_assert(
            std::is_convertible_v<TRoomFeatureRequest *, Features::IRoomFeatureRequestBase *>,
            "TRoomFeatureRequest must inherit from IRoomFeatureRequest.");
        checkf(
            FeatureRequest->IsValid(),
            TEXT("The feature request passed to AddFeatureRequest<> does not have a valid state."));
        this->FeatureRequests.Add(FeatureRequest->GetFeatureName(), FeatureRequest);
    }

    /**
     * Returns whether the specified feature has been requested by calling AddFeatureRequest.
     */
    template <typename TRoomFeatureRequest> bool HasFeatureRequest() const
    {
        static_assert(
            std::is_convertible_v<TRoomFeatureRequest *, Features::IRoomFeatureRequestBase *>,
            "TRoomFeatureRequest must inherit from IRoomFeatureRequest.");
        return this->FeatureRequests.Contains(TRoomFeatureRequest::GetFeatureNameStatic());
    }

    /**
     * Returns the feature request if it has been requested by calling AddFeatureRequest.
     */
    template <typename TRoomFeatureRequest> TSharedPtr<const TRoomFeatureRequest> GetFeatureRequest() const
    {
        static_assert(
            std::is_convertible_v<TRoomFeatureRequest *, Features::IRoomFeatureRequestBase *>,
            "TRoomFeatureRequest must inherit from IRoomFeatureRequest.");
        if (this->FeatureRequests.Contains(TRoomFeatureRequest::GetFeatureNameStatic()))
        {
            return StaticCastSharedPtr<TRoomFeatureRequest>(
                this->FeatureRequests[TRoomFeatureRequest::GetFeatureNameStatic()]);
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * Returns a feature request previously requested by calling AddFeatureRequest, or asserts if the feature wasn't
     * requested.
     */
    template <typename TRoomFeatureRequest> TSharedRef<const TRoomFeatureRequest> GetFeatureRequestChecked() const
    {
        static_assert(
            std::is_convertible_v<TRoomFeatureRequest *, Features::IRoomFeatureRequestBase *>,
            "TRoomFeatureRequest must inherit from IRoomFeatureRequest.");
        return StaticCastSharedRef<TRoomFeatureRequest>(
            this->FeatureRequests[TRoomFeatureRequest::GetFeatureNameStatic()].ToSharedRef());
    }
};

}

namespace Redpoint::EOS::Rooms::Requests
{
REDPOINT_EOS_FILE_NS_EXPORT(212611124, Redpoint::EOS::Rooms::Requests, FFeatureRequestsOnRequest)
}

REDPOINT_EOS_CODE_GUARD_END()