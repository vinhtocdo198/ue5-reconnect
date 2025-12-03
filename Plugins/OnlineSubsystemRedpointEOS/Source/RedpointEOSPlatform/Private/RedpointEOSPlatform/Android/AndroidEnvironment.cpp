// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatform/Android/AndroidEnvironment.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if PLATFORM_ANDROID

namespace REDPOINT_EOS_FILE_NS_ID(1687685427, Redpoint::EOS::Platform::Android)
{

bool FAndroidEnvironment::IsMetaQuest()
{
    static bool bIsChecked = false;
    static bool bIsMeta = false;

    if (!bIsChecked)
    {
        JNIEnv *Env = FAndroidApplication::GetJavaEnv();
        checkf(Env != nullptr, TEXT("JavaEnv must be available on module startup!"));
        {
            jmethodID RedpointIsOculusMethod = FJavaWrapper::FindMethod(
                Env,
                FJavaWrapper::GameActivityClassID,
                "Thunk_Redpoint_IsOculus",
                "()Z",
                true);
            bIsMeta = FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, RedpointIsOculusMethod);
            bIsChecked = true;
        }
    }

    return bIsMeta;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()