// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Socket/PacketTiming.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
#include "Containers/Array.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Logging.h"
#include "Serialization/Archive.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(3354339269, Redpoint::EOS::Networking::Socket)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;

static TArray<TSharedRef<FPacketTiming>> PacketTimingBuffer;

static TUniquePtr<FArchive> PacketTimingBufferFile;

FPacketTiming::FPacketTiming(
    const EOS_ProductUserId &InSourceUserId,
    const EOS_ProductUserId &InDestinationUserId,
    const FSocketId &InSymmetricSocketId,
    uint8_t InSymmetricChannel,
    int64 SentTicks,
    const FDateTime &ReceivedAt,
    int32 InSize)
    : bHasTimingData(true)
    , SourceUserId(GetUserIdString(GetAccountId(InSourceUserId)))
    , DestinationUserId(GetUserIdString(GetAccountId(InDestinationUserId)))
    , SymmetricSocketId(InSymmetricSocketId)
    , SymmetricChannel(InSymmetricChannel)
    , Sent(FDateTime(SentTicks))
    , ReceivedIntoQueue(ReceivedAt)
    , PulledFromQueue(FDateTime::MinValue())
    , Size(InSize)
{
}

static void WritePacketTimingString(const FString &InString)
{
    if (PacketTimingBufferFile == nullptr)
    {
        FString SavedPath = FPaths::ProjectSavedDir() / TEXT("EOSPacketTiming.csv");
        PacketTimingBufferFile =
            TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(*SavedPath, FILEWRITE_AllowRead));

        if (PacketTimingBufferFile == nullptr)
        {
            UE_LOG(
                LogRedpointEOSNetworking,
                Error,
                TEXT("Unable to write packet tracing file to path: %s"),
                *SavedPath);
        }
        else
        {
            UE_LOG(LogRedpointEOSNetworking, Verbose, TEXT("Writing packet tracing to: %s"), *SavedPath);
        }
    }

    if (PacketTimingBufferFile != nullptr)
    {
        FTCHARToUTF8 UTF8String(*InString);
        PacketTimingBufferFile->Serialize(
            const_cast<UTF8CHAR *>(reinterpret_cast<const UTF8CHAR *>(UTF8String.Get())),
            static_cast<int64>(static_cast<SIZE_T>(UTF8String.Length()) * sizeof(UTF8CHAR)));
    }
}

void RecordPacketTiming(const TSharedPtr<FPacketTiming> &InPacketTiming)
{
    if (CVarEnablePacketTiming.GetValueOnGameThread())
    {
        if (!InPacketTiming.IsValid())
        {
            return;
        }

        // If PulledFromQueue is MinValue, then this packet is being
        // acted upon right now (e.g. control channel packets).
        if (InPacketTiming->PulledFromQueue == FDateTime::MinValue())
        {
            InPacketTiming->PulledFromQueue = FDateTime::UtcNow();
        }

        if (PacketTimingBufferFile == nullptr)
        {
            WritePacketTimingString(TEXT(
                "Src User,Dest User,Socket Name,Channel,Bytes Sent,Sent At,Received At,Dequeued At,In Flight micros,In "
                "Queue micros,Latency micros,Latency ms\n"));
        }

        // PacketTimingBufferFile can still be null if the file can't be opened for writing.
        if (PacketTimingBufferFile != nullptr)
        {
            PacketTimingBuffer.Add(InPacketTiming.ToSharedRef());

            if (PacketTimingBuffer.Num() > 100)
            {
                for (const auto &Entry : PacketTimingBuffer)
                {
                    WritePacketTimingString(FString::Printf(
                        TEXT("%s,%s,%s,%u,%d,%lld,%lld,%lld,%f,%f,%f,%f\n"),
                        *Entry->SourceUserId,
                        *Entry->DestinationUserId,
                        *Entry->SymmetricSocketId,
                        Entry->SymmetricChannel,
                        Entry->Size,
                        Entry->Sent.GetTicks(),
                        Entry->ReceivedIntoQueue.GetTicks(),
                        Entry->PulledFromQueue.GetTicks(),
                        (Entry->ReceivedIntoQueue - Entry->Sent).GetTotalMicroseconds(),
                        (Entry->PulledFromQueue - Entry->ReceivedIntoQueue).GetTotalMicroseconds(),
                        (Entry->PulledFromQueue - Entry->Sent).GetTotalMicroseconds(),
                        (Entry->PulledFromQueue - Entry->Sent).GetTotalMilliseconds()));
                }

                PacketTimingBufferFile->Flush();
                PacketTimingBuffer.Empty();
            }
        }
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()
