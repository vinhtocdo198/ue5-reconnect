// Copyright June Rhodes. All Rights Reserved.

// Parts of this file are based on AsyncTaskDownloadImage.cpp.

#include "RedpointEOSCore/Utils/TextureLoader.h"

#if REDPOINT_EOS_HAS_IMAGE_DECODING
#include "Engine/Texture2DDynamic.h"
#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "RenderingThread.h"
#include "TextureResource.h"
#endif
#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1029236471, Redpoint::EOS::Core::Utils)
{

#if REDPOINT_EOS_HAS_IMAGE_DECODING

static void WriteRawToTexture_RenderThread(
    FTexture2DDynamicResource *TextureResource,
    TArray64<uint8> *RawData,
    bool bUseSRGB = true)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    check(IsInRenderingThread());

    if (TextureResource)
    {
#if REDPOINT_EOS_UE_5_5_OR_LATER
        FRHITexture *TextureRHI = TextureResource->GetTexture2DRHI();
#else
        FRHITexture2D *TextureRHI = TextureResource->GetTexture2DRHI();
#endif

        int32 Width = TextureRHI->GetDesc().Extent.X;
        int32 Height = TextureRHI->GetDesc().Extent.Y;

        uint32 DestStride = 0;
        uint8 *DestData =
            reinterpret_cast<uint8 *>(RHILockTexture2D(TextureRHI, 0, RLM_WriteOnly, DestStride, false, false));

        for (int32 y = 0; y < Height; y++)
        {
            uint8 *DestPtr = &DestData[((int64)Height - 1 - y) * DestStride];

            const FColor *SrcPtr = &((FColor *)(void *)(RawData->GetData()))[((int64)Height - 1 - y) * Width];
            for (int32 x = 0; x < Width; x++)
            {
                *DestPtr++ = SrcPtr->B;
                *DestPtr++ = SrcPtr->G;
                *DestPtr++ = SrcPtr->R;
                *DestPtr++ = SrcPtr->A;
                SrcPtr++;
            }
        }

        RHIUnlockTexture2D(TextureRHI, 0, false, false);
    }

    delete RawData;
}

#endif

TSoftObjectPtr<UTexture> FTextureLoader::LoadTextureFromHttpResponse(const FHttpResponseRef &Response)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

#if REDPOINT_EOS_HAS_IMAGE_DECODING
    if (Response->GetContentLength() == 0)
    {
        return nullptr;
    }

    EImageFormat ImageFormat = EImageFormat::Invalid;
    if (Response->GetHeader("Content-Type") == "image/jpeg")
    {
        ImageFormat = EImageFormat::JPEG;
    }
    else if (Response->GetHeader("Content-Type") == "image/png")
    {
        ImageFormat = EImageFormat::PNG;
    }
    else
    {
        return nullptr;
    }

    IImageWrapperModule &ImageWrapperModule =
        FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
    if (ImageWrapper.IsValid() &&
        ImageWrapper->SetCompressed(Response->GetContent().GetData(), static_cast<int64>(Response->GetContentLength())))
    {
        TArray64<uint8> *RawData = new TArray64<uint8>();
        const ERGBFormat InFormat = ERGBFormat::BGRA;
        if (ImageWrapper->GetRaw(InFormat, 8, *RawData))
        {
            if (UTexture2DDynamic *Texture =
                    UTexture2DDynamic::Create((int32)ImageWrapper->GetWidth(), (int32)ImageWrapper->GetHeight()))
            {
                Texture->SRGB = true;
                Texture->UpdateResource();

                FTexture2DDynamicResource *TextureResource =
                    static_cast<FTexture2DDynamicResource *>(Texture->GetResource());
                if (TextureResource)
                {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-member-function"
#endif
                    ENQUEUE_RENDER_COMMAND(FWriteRawDataToTexture)
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
                    ([TextureResource, RawData](FRHICommandListImmediate &RHICmdList) {
                        WriteRawToTexture_RenderThread(TextureResource, RawData);
                    });
                }
                else
                {
                    delete RawData;
                }

                return Texture;
            }
        }
    }
#endif

    return nullptr;
}

void FTextureLoader::LoadTextureFromHttpUrl(const FString &Url, const FOnTextureLoadComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

#if REDPOINT_EOS_HAS_IMAGE_DECODING
    auto Request = FHttpModule::Get().CreateRequest();
    Request->SetVerb("GET");
    Request->SetURL(Url);
    Request->OnProcessRequestComplete().BindLambda(
        [OnComplete](FHttpRequestPtr, FHttpResponsePtr Response, bool bProcessedSuccessfully) {
            if (!bProcessedSuccessfully || !Response.IsValid())
            {
                OnComplete.ExecuteIfBound(nullptr);
                return;
            }

            OnComplete.ExecuteIfBound(FTextureLoader::LoadTextureFromHttpResponse(Response.ToSharedRef()));
        });
    Request->ProcessRequest();
#else
    OnComplete.ExecuteIfBound(nullptr);
#endif
}

}

REDPOINT_EOS_CODE_GUARD_END()