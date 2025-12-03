// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/RTCAudio/DeviceInformation.h"
#include "RedpointEOSAPI/Templates/Decl/AsyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

class REDPOINTEOSAPI_API FQueryInputDevicesInformation
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_BEGIN(
        RTCAudio,
        QueryInputDevicesInformation,
        EOS_RTCAUDIO_QUERYINPUTDEVICESINFORMATION_API_LATEST,
        GetInputDevicesCount,
        EOS_RTCAUDIO_GETINPUTDEVICESCOUNT_API_LATEST,
        CopyInputDeviceInformationByIndex,
        EOS_RTCAUDIO_COPYINPUTDEVICEINFORMATIONBYINDEX_API_LATEST,
        InputDeviceInformation)

    class Options
    {
    };

    typedef FDeviceInformation ResultEntry;

    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()