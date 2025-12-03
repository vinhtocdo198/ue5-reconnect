// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/RTCAudio/DeviceInformation.h"
#include "RedpointEOSAPI/Templates/Decl/AsyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

class REDPOINTEOSAPI_API FQueryOutputDevicesInformation
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_BEGIN(
        RTCAudio,
        QueryOutputDevicesInformation,
        EOS_RTCAUDIO_QUERYOUTPUTDEVICESINFORMATION_API_LATEST,
        GetOutputDevicesCount,
        EOS_RTCAUDIO_GETOUTPUTDEVICESCOUNT_API_LATEST,
        CopyOutputDeviceInformationByIndex,
        EOS_RTCAUDIO_COPYOUTPUTDEVICEINFORMATIONBYINDEX_API_LATEST,
        OutputDeviceInformation)

    class Options
    {
    };

    typedef FDeviceInformation ResultEntry;

    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()