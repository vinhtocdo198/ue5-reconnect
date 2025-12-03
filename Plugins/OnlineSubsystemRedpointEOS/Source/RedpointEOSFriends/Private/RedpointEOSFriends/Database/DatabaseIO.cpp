// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/Database/DatabaseIO.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSFriends/Logging.h"
#include "RedpointEOSStorage/StorageSystem.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(522457488, Redpoint::EOS::Friends::Database)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Friends;
using namespace ::Redpoint::EOS::Storage;

static auto RedpointEOSFriendsDatabaseHeader = TArray<uint8>{'F', 'R', 'I', 'E', 'N', 'D', 'D', 'B', '1'};

void FDatabaseIO::LoadFriendDatabase(
    const FPlatformHandle &PlatformHandle,
    const UE::Online::FAccountId &LocalUserId,
    const FOnFriendDatabaseLoaded &OnLoaded)
{
    auto StorageSystem = PlatformHandle->GetSystem<IStorageSystem>();
    StorageSystem->List(
        LocalUserId,
        EStorageType::PlayerDataStorage,
        TArray<FString>{TEXT("__AllowRedpointInternalFiles__")},
        IStorageSystem::FOnListComplete::CreateLambda(
            [StorageSystem, LocalUserId, OnLoaded](const FError &ListErrorCode, const TArray<FFileEntry> &FileList) {
                if (!ListErrorCode.WasSuccessful())
                {
                    auto ListErrorCodeWithContext =
                        ListErrorCode.WithContext(TEXT("Failed to list Player Data Storage files."));
                    UE_LOG(LogRedpointEOSFriends, Error, TEXT("%s"), *ListErrorCodeWithContext.ToLogString());
                    OnLoaded.ExecuteIfBound(ListErrorCodeWithContext, nullptr);
                    return;
                }

                bool bFoundDatabase = false;
                for (const auto &File : FileList)
                {
                    if (File.FilePath == TEXT("__EOS__/PDSFriends.db"))
                    {
                        bFoundDatabase = true;
                    }
                }
                if (!bFoundDatabase)
                {
                    auto ResultError = Errors::NotFound(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("No cross-platform friends database currently exists for this user."));
                    if (!GIsAutomationTesting)
                    {
                        UE_LOG(LogRedpointEOSFriends, Warning, TEXT("%s"), *ResultError.ToLogString());
                    }
                    OnLoaded.ExecuteIfBound(ResultError, nullptr);
                    return;
                }

                StorageSystem->Read(
                    LocalUserId,
                    EStorageType::PlayerDataStorage,
                    TEXT("__EOS__/PDSFriends.db"),
                    IStorageSystem::FOnReadComplete::CreateLambda([
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                                                      LocalUserId,
#endif
                                                                      OnLoaded](
                                                                      const FError &ReadErrorCode,
                                                                      const TArray<uint8> &FileData) {
                        if (!ReadErrorCode.WasSuccessful())
                        {
                            auto ReadErrorCodeWithContext = ReadErrorCode.WithContext(
                                TEXT("Failed to read '__EOS__/PDSFriends.db' Player Data Storage file"));
                            UE_LOG(LogRedpointEOSFriends, Error, TEXT("%s"), *ReadErrorCodeWithContext.ToLogString());
                            OnLoaded.ExecuteIfBound(ReadErrorCodeWithContext, nullptr);
                            return;
                        }

                        auto HeaderSize = RedpointEOSFriendsDatabaseHeader.Num() + FSHA1::DigestSize;
                        if (FileData.Num() < HeaderSize)
                        {
                            auto ResultError = Errors::InvalidState(
                                ANSI_TO_TCHAR(__FUNCTION__),
                                TEXT("The cross-platform friends database is corrupt (file too short)."));
                            UE_LOG(LogRedpointEOSFriends, Error, TEXT("%s"), *ResultError.ToLogString());
                            OnLoaded.ExecuteIfBound(ResultError, nullptr);
                            return;
                        }

                        bool bHasMagicSignature = true;
                        for (int i = 0; i < RedpointEOSFriendsDatabaseHeader.Num(); i++)
                        {
                            if (FileData[i] != RedpointEOSFriendsDatabaseHeader[i])
                            {
                                bHasMagicSignature = false;
                                break;
                            }
                        }
                        if (!bHasMagicSignature)
                        {
                            auto ResultError = Errors::InvalidState(
                                ANSI_TO_TCHAR(__FUNCTION__),
                                TEXT("The cross-platform friends database is corrupt (missing magic signature)."));
                            UE_LOG(LogRedpointEOSFriends, Error, TEXT("%s"), *ResultError.ToLogString());
                            OnLoaded.ExecuteIfBound(ResultError, nullptr);
                            return;
                        }

                        FSHAHash Hash = FSHA1::HashBuffer(
                            FileData.GetData() + HeaderSize,
                            static_cast<uint64>(FileData.Num() - HeaderSize));
                        for (int i = 0; i < FSHA1::DigestSize; i++)
                        {
                            if (Hash.Hash[i] != FileData[RedpointEOSFriendsDatabaseHeader.Num() + i])
                            {
                                auto ResultError = Errors::InvalidState(
                                    ANSI_TO_TCHAR(__FUNCTION__),
                                    TEXT("The cross-platform friends database is corrupt (mismatching SHA1 hash)."));
                                UE_LOG(LogRedpointEOSFriends, Error, TEXT("%s"), *ResultError.ToLogString());
                                OnLoaded.ExecuteIfBound(ResultError, nullptr);
                                return;
                            }
                        }

                        UE_LOG(
                            LogRedpointEOSFriends,
                            Verbose,
                            TEXT("Load of cross-platform friends database for local user ID '%s' passed hash check."),
                            *GetUserIdDebugString(LocalUserId));

                        TArrayView<const uint8> FileDataAfterHeader =
                            MakeArrayView(FileData.GetData() + HeaderSize, FileData.Num() - HeaderSize);
                        FMemoryReaderView Reader(FileDataAfterHeader, true);
                        auto FriendDatabase = MakeShared<FSerializedFriendDatabase>();
                        FriendDatabase->Archive(Reader);

                        UE_LOG(
                            LogRedpointEOSFriends,
                            Verbose,
                            TEXT("Loaded cross-platform friends database has %d runtime integrations cached."),
                            FriendDatabase->RuntimeIntegrationCachedFriends.Num());
                        for (const auto &KV : FriendDatabase->RuntimeIntegrationCachedFriends)
                        {
                            UE_LOG(
                                LogRedpointEOSFriends,
                                Verbose,
                                TEXT("Loaded cross-platform friends database has %d friends cached for runtime "
                                     "integration cache key '%s'."),
                                KV.Value.Num(),
                                *KV.Key);
                        }

                        OnLoaded.ExecuteIfBound(Errors::Success(), FriendDatabase);
                        return;
                    }));
            }));
}

void FDatabaseIO::SaveFriendDatabase(
    const FPlatformHandle &PlatformHandle,
    const UE::Online::FAccountId &LocalUserId,
    const TSharedRef<FSerializedFriendDatabase> &FriendDatabase,
    bool bRequireNoDatabaseFilePresent,
    const FOnFriendDatabaseSaved &OnSaved)
{
    auto StorageSystem = PlatformHandle->GetSystem<IStorageSystem>();
    StorageSystem->List(
        LocalUserId,
        EStorageType::PlayerDataStorage,
        TArray<FString>{TEXT("__AllowRedpointInternalFiles__")},
        IStorageSystem::FOnListComplete::CreateLambda(
            [StorageSystem, LocalUserId, FriendDatabase, OnSaved, bRequireNoDatabaseFilePresent](
                const FError &ListErrorCode,
                const TArray<FFileEntry> &FileList) {
                if (!ListErrorCode.WasSuccessful())
                {
                    auto ListErrorCodeWithContext =
                        ListErrorCode.WithContext(TEXT("Failed to list Player Data Storage files"));
                    UE_LOG(LogRedpointEOSFriends, Error, TEXT("%s"), *ListErrorCodeWithContext.ToLogString());
                    OnSaved.ExecuteIfBound(ListErrorCodeWithContext);
                    return;
                }

                bool bFoundDatabase = false;
                for (const auto &File : FileList)
                {
                    if (File.FilePath == TEXT("__EOS__/PDSFriends.db"))
                    {
                        bFoundDatabase = true;
                    }
                }
                if (bFoundDatabase && bRequireNoDatabaseFilePresent)
                {
                    // When we fail to load the database (potentially due to a transient error on the backend), we want
                    // to ensure that we don't then later overwrite the real database with an almost-blank version. This
                    // check prevents that.
                    auto ResultError = Errors::AccessDenied(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Expected there to be no cross-platform friends database on the backend for this user, "
                             "but one was found."));
                    UE_LOG(LogRedpointEOSFriends, Error, TEXT("%s"), *ResultError.ToLogString());
                    OnSaved.ExecuteIfBound(ResultError);
                    return;
                }

                TArray<uint8> FileDataAfterHeader;
                FMemoryWriter Writer(FileDataAfterHeader, true);
                FriendDatabase->Archive(Writer);
                Writer.Flush();

                FSHAHash Hash =
                    FSHA1::HashBuffer(FileDataAfterHeader.GetData(), static_cast<uint64>(FileDataAfterHeader.Num()));

                TArray<uint8> FileData;
                FileData.Empty(RedpointEOSFriendsDatabaseHeader.Num() + FSHA1::DigestSize + FileDataAfterHeader.Num());
                for (int i = 0; i < RedpointEOSFriendsDatabaseHeader.Num(); i++)
                {
                    FileData.Add(RedpointEOSFriendsDatabaseHeader[i]);
                }
                for (int i = 0; i < FSHA1::DigestSize; i++)
                {
                    FileData.Add(Hash.Hash[i]);
                }
                for (int i = 0; i < FileDataAfterHeader.Num(); i++)
                {
                    FileData.Add(FileDataAfterHeader[i]);
                }

                UE_LOG(
                    LogRedpointEOSFriends,
                    Verbose,
                    TEXT("Saving cross-platform friends database with %d runtime integrations cached."),
                    FriendDatabase->RuntimeIntegrationCachedFriends.Num());
                for (const auto &KV : FriendDatabase->RuntimeIntegrationCachedFriends)
                {
                    UE_LOG(
                        LogRedpointEOSFriends,
                        Verbose,
                        TEXT("Saving cross-platform friends database has %d friends cached for runtime "
                             "integration cache key '%s'."),
                        KV.Value.Num(),
                        *KV.Key);
                }

                StorageSystem->Write(
                    LocalUserId,
                    TEXT("__EOS__/PDSFriends.db"),
                    FileData,
                    IStorageSystem::FOnWriteComplete::CreateLambda([
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                                                       LocalUserId,
#endif
                                                                       OnSaved](const FError &WriteErrorCode) {
                        if (!WriteErrorCode.WasSuccessful())
                        {
                            UE_LOG(LogRedpointEOSFriends, Error, TEXT("%s"), *WriteErrorCode.ToLogString());
                            OnSaved.ExecuteIfBound(WriteErrorCode);
                            return;
                        }

                        UE_LOG(
                            LogRedpointEOSFriends,
                            Verbose,
                            TEXT("Save of cross-platform friends database for local user ID '%s' was successful."),
                            *GetUserIdDebugString(LocalUserId));
                        OnSaved.ExecuteIfBound(Errors::Success());
                        return;
                    }));
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()