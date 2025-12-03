// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/DevAuthToolLauncher.h"

#if PLATFORM_WINDOWS || PLATFORM_MAC
#include "HAL/PlatformFileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSConfig/IniConfig.h"
#include "RedpointEOSCore/Editor/RedpointEOSSDKInfo.h"
#include "RedpointEOSEditorModule.h"
#include "libzip/zip.h"
#if !PLATFORM_WINDOWS
#include <sys/stat.h>
#include <unistd.h>
#endif
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if PLATFORM_WINDOWS || PLATFORM_MAC

#define LOCTEXT_NAMESPACE "OnlineSubsystemEOSEditorModule"

namespace REDPOINT_EOS_FILE_NS_ID(314590786, Redpoint::EOS::Editor)
{
using namespace ::Redpoint::EOS::Core::Editor;

FDevAuthToolLauncher FDevAuthToolLauncher::Instance;

FDevAuthToolLauncher &FDevAuthToolLauncher::Get()
{
    return FDevAuthToolLauncher::Instance;
}

FDevAuthToolLauncher::FDevAuthToolLauncher()
    : bNeedsToCheckIfDevToolRunning(true)
    , bIsDevToolRunning()
    , ResetCheckHandle()
{
}

bool FDevAuthToolLauncher::ClearKnowledgeThatDevAuthToolIsRunning(float)
{
    FDevAuthToolLauncher::Get().bNeedsToCheckIfDevToolRunning = true;
    return false;
}

FString FDevAuthToolLauncher::GetShortestPathFromRelativePath(const FString &RelativePath)
{
    FString AbsolutePath = FPaths::ConvertRelativePathToFull(RelativePath);

    if (AbsolutePath.Len() < RelativePath.Len())
    {
        return AbsolutePath;
    }
    else
    {
        return RelativePath;
    }
}

void FDevAuthToolLauncher::ScheduleClearKnowledgeThatDevAuthToolIsRunning()
{
    ResetCheckHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateStatic(&FDevAuthToolLauncher::ClearKnowledgeThatDevAuthToolIsRunning),
        10.0f);
}

FString FDevAuthToolLauncher::GetPathToToolsFolder()
{
    auto EOSSDKInfo = FRedpointEOSSDKInfo::Get();

    TArray<FString> ZipNames;
    for (const auto &Version : EOSSDKInfo.SupportedVersions(TEXT("DevAuthTool")))
    {
#if PLATFORM_WINDOWS
        ZipNames.Add(FString::Printf(TEXT("EOS_DevAuthTool-win32-x64-%s.zip"), *Version));
#else
        ZipNames.Add(FString::Printf(TEXT("EOS_DevAuthTool-darwin-x64-%s.zip"), *Version));
#endif
    }

    for (const auto &SearchPathWithoutTools : EOSSDKInfo.SearchPaths(TEXT("EOSSDK")))
    {
        auto SearchPath = SearchPathWithoutTools / TEXT("Tools");
#if PLATFORM_WINDOWS
        if (FPaths::FileExists(SearchPath / TEXT("DevAuthTool") / TEXT("EOS_DevAuthTool.exe")))
        {
            return GetShortestPathFromRelativePath(SearchPath.Replace(TEXT("/"), TEXT("\\")));
        }
        for (const auto &ZipName : ZipNames)
        {
            if (FPaths::FileExists(SearchPath / ZipName))
            {
                return GetShortestPathFromRelativePath(SearchPath.Replace(TEXT("/"), TEXT("\\")));
            }
        }
#else
        if (FPaths::FileExists(
                SearchPath / TEXT("EOS_DevAuthTool.app") / TEXT("Contents") / TEXT("MacOS") / TEXT("EOS_DevToolApp")))
        {
            return SearchPath;
        }
        for (const auto &ZipName : ZipNames)
        {
            if (FPaths::FileExists(SearchPath / ZipName))
            {
                return SearchPath;
            }
        }
#endif
    }

    return TEXT("");
}

template <typename T> class TAutoCleanup
{
private:
    T *Ptr;
    TFunction<void(T *)> Cleanup;

public:
    TAutoCleanup(T *InPtr, TFunction<void(T *)> InCleanup)
        : Ptr(InPtr)
        , Cleanup(InCleanup)
    {
    }
    UE_NONCOPYABLE(TAutoCleanup)
    ~TAutoCleanup()
    {
        this->Destroy();
    }
    void Destroy()
    {
        if (this->Ptr != nullptr)
        {
            this->Cleanup(this->Ptr);
            this->Ptr = nullptr;
        }
    }
};

static void ExtractLink(
    zip_t *ZipFile,
    zip_uint64_t ZipIndex,
    const zip_stat_t &ZipStat,
    const FString &ZipFileName,
    const FString &DestinationRoot)
{
    auto &FM = IFileManager::Get();
    FM.MakeDirectory(*FPaths::GetPath(DestinationRoot / ZipFileName), true);

    TArray<uint8> Buffer;
#if REDPOINT_EOS_UE_5_4_OR_LATER
    Buffer.SetNumUninitialized(static_cast<int32>(ZipStat.size), EAllowShrinking::No);
#else
    Buffer.SetNumUninitialized(static_cast<int32>(ZipStat.size), false);
#endif
    zip_file *EmbeddedFile = zip_fopen_index(ZipFile, ZipIndex, 0 /* flags */);
    if (!EmbeddedFile)
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Unable to read file '%s' located within the ZIP archive. This file will not be extracted."),
            *ZipFileName);
        return;
    }
    bool bReadSuccess =
        static_cast<zip_uint64_t>(zip_fread(EmbeddedFile, Buffer.GetData(), ZipStat.size)) == ZipStat.size;
    zip_fclose(EmbeddedFile);
    if (!bReadSuccess)
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Unable to fully read file '%s' located within the ZIP archive. This file will not be extracted."),
            *ZipFileName);
        return;
    }

    FString Target = FString(static_cast<int32>(ZipStat.size), reinterpret_cast<ANSICHAR *>(Buffer.GetData()));

#if PLATFORM_WINDOWS
    UE_LOG(
        LogRedpointEOSEditor,
        Warning,
        TEXT("This platform does not support creating symbolic links; unable to create symbolic link at '%s' to target "
             "'%s'."),
        *(DestinationRoot / ZipFileName),
        *Target);
#else
    int SymlinkResult = symlink(TCHAR_TO_ANSI(*Target), TCHAR_TO_ANSI(*(DestinationRoot / ZipFileName)));
    if (SymlinkResult != 0)
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Verbose,
            TEXT("Failed to create symbolic link at '%s' to target '%s', got result code %d."),
            *(DestinationRoot / ZipFileName),
            *Target,
            SymlinkResult);
    }
#endif
}

static void ExtractFile(
    zip_t *ZipFile,
    zip_uint64_t ZipIndex,
    const zip_stat_t &ZipStat,
    const FString &ZipFileName,
    const FString &DestinationRoot)
{
    auto &FM = IFileManager::Get();
    FM.MakeDirectory(*FPaths::GetPath(DestinationRoot / ZipFileName), true);

    TArray<uint8> Buffer;
#if REDPOINT_EOS_UE_5_4_OR_LATER
    Buffer.SetNumUninitialized(static_cast<int32>(ZipStat.size), EAllowShrinking::No);
#else
    Buffer.SetNumUninitialized(static_cast<int32>(ZipStat.size), false);
#endif
    zip_file *EmbeddedFile = zip_fopen_index(ZipFile, ZipIndex, 0 /* flags */);
    if (!EmbeddedFile)
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Unable to read file '%s' located within the ZIP archive. This file will not be extracted."),
            *ZipFileName);
        return;
    }
    bool bReadSuccess =
        static_cast<zip_uint64_t>(zip_fread(EmbeddedFile, Buffer.GetData(), ZipStat.size)) == ZipStat.size;
    zip_fclose(EmbeddedFile);
    if (!bReadSuccess)
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Unable to fully read file '%s' located within the ZIP archive. This file will not be extracted."),
            *ZipFileName);
        return;
    }

    if (!FFileHelper::SaveArrayToFile(Buffer, *(DestinationRoot / ZipFileName)))
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Unable to extract file '%s' from the Developer Authentication Tool ZIP to '%s'."),
            *ZipFileName,
            *(DestinationRoot / ZipFileName));
        return;
    }
}

bool FDevAuthToolLauncher::ExtractDeveloperAuthenticationTool(const FString &Path, bool bInteractive)
{
    if (IFileManager::Get().IsReadOnly(*Path))
    {
        if (!FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*Path, false))
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Error,
                TEXT("Unable to mark the following directory as writable, which is necessary to extract the Developer "
                     "Authentication Tool: %s"),
                *Path);
            return false;
        }
    }

#if PLATFORM_WINDOWS
    FString DestinationPath = Path / TEXT("DevAuthTool");
#else
    FString DestinationPath = Path;
#endif

    // Remove any existing version of the tool (in case we are re-extracting it because it is corrupted).
#if PLATFORM_WINDOWS
    IFileManager::Get().DeleteDirectory(*DestinationPath, false, true);
#else
    IFileManager::Get().DeleteDirectory(*(DestinationPath / TEXT("EOS_DevAuthTool.app")), false, true);
#endif

    // Determine our search paths for ZIP files that contains the Developer Authentication Tool.
    auto EOSSDKInfo = FRedpointEOSSDKInfo::Get();
    TArray<FString> ZipNames;
    for (const auto &Version : EOSSDKInfo.SupportedVersions(TEXT("DevAuthTool")))
    {
#if PLATFORM_WINDOWS
        ZipNames.Add(FString::Printf(TEXT("EOS_DevAuthTool-win32-x64-%s.zip"), *Version));
#else
        ZipNames.Add(FString::Printf(TEXT("EOS_DevAuthTool-darwin-x64-%s.zip"), *Version));
#endif
    }
    TArray<FString> ZipPaths;
    for (const auto &ZipName : ZipNames)
    {
        ZipPaths.Add(Path / ZipName);
    }
    FString FoundZip = TEXT("");
    for (const auto &ZipPath : ZipPaths)
    {
        if (FPaths::FileExists(ZipPath))
        {
            FoundZip = ZipPath;
        }
    }
    if (FoundZip.IsEmpty())
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Error,
            TEXT("Unable to locate the ZIP file that contains the Developer Authentication Tool in the EOS SDK "
                 "Tools folder."));
        return false;
    }

    // Initialize ZIP error structure.
    zip_error_t ZipError = {};
    zip_error_init(&ZipError);
    TAutoCleanup<zip_error_t> ZipErrorCleanup(&ZipError, [](zip_error_t *Ptr) {
        zip_error_fini(Ptr);
    });

    // Open the ZIP file directly.
    int ZipFileError = 0;
    zip_t *ZipFile = zip_open(TCHAR_TO_ANSI(*FoundZip), ZIP_RDONLY, &ZipFileError);
    if (ZipFile == nullptr)
    {
        zip_error_init_with_code(&ZipError, ZipFileError);
        UE_LOG(
            LogRedpointEOSEditor,
            Error,
            TEXT("Unable to open the ZIP file that contains the Developer Authentication Tool in the EOS SDK Tools "
                 "folder for reading (%hs)."),
            zip_error_strerror(&ZipError));
        return false;
    }
    TAutoCleanup<zip_t> ZipFileCleanup(ZipFile, [](zip_t *Ptr) {
        zip_close(Ptr);
    });

    // Get the total number of entries in the ZIP file.
    zip_int64_t NumberOfFiles = zip_get_num_entries(ZipFile, 0);
    if (NumberOfFiles < 0 || MAX_int32 < NumberOfFiles)
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Error,
            TEXT("Unable to read the ZIP file that contains the Developer Authentication Tool in the EOS SDK Tools "
                 "(invalid number of embedded files in the ZIP)."));
        return false;
    }

    // Iterate through the entries and perform the extraction (we don't bother making an index like FZipArchiveReader).
    for (zip_uint64_t i = 0; i < static_cast<uint64_t>(NumberOfFiles); i++)
    {
        zip_stat_t ZipFileStat = {};
        if (zip_stat_index(ZipFile, i, 0, &ZipFileStat) != 0)
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Error,
                TEXT("Unable to read the ZIP file that contains the Developer Authentication Tool in the EOS SDK Tools "
                     "(unable to read stat of file at index %d)."),
                i);
            return false;
        }

        zip_uint64_t ValidStat = ZipFileStat.valid;
        if (!(ValidStat & ZIP_STAT_NAME))
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Error,
                TEXT("Unable to read the ZIP file that contains the Developer Authentication Tool in the EOS SDK Tools "
                     "(stat for embedded file at index %d does not include name)."),
                i);
            return false;
        }

        auto ZipFileName = FString(ANSI_TO_TCHAR(ZipFileStat.name));
        if (ZipFileName.EndsWith(TEXT("/")) || ZipFileName.EndsWith(TEXT("\\")))
        {
            // This is a directory; skip it as we'll automatically create intermediate directories as needed.
            continue;
        }

        // Each ZIP file entry contains external attributes of *only* one 'opsys' type. Therefore this function gets
        // both the opsys and the attributes.
        zip_uint8_t OpSys = 0;
        zip_uint32_t Attr = 0;
        zip_file_get_external_attributes(ZipFile, i, ZIP_FL_UNCHANGED, &OpSys, &Attr);
        if (OpSys == ZIP_OPSYS_UNIX)
        {
            Attr = ((Attr) >> 16L);

            auto Perm = (Attr & ~0170000u);
            auto Mode = (Attr & 0170000u);

            bool IsLink = (Mode == 0120000u);

            UE_LOG(
                LogRedpointEOSEditor,
                Verbose,
                TEXT("ZIP index %d: name='%s', opsys=%u (unix), attr=%u, link=%s, perm=%u, mode=%u."),
                i,
                *ZipFileName,
                OpSys,
                Attr,
                IsLink ? TEXT("yes") : TEXT("no"),
                Perm,
                Mode);
            if (IsLink)
            {
                ExtractLink(ZipFile, i, ZipFileStat, ZipFileName, DestinationPath);
            }
            else
            {
                ExtractFile(ZipFile, i, ZipFileStat, ZipFileName, DestinationPath);
            }
#if !PLATFORM_WINDOWS
            auto ChmodResult = chmod(TCHAR_TO_ANSI(*(DestinationPath / ZipFileName)), static_cast<mode_t>(Mode));
            if (ChmodResult != 0)
            {
                UE_LOG(
                    LogRedpointEOSEditor,
                    Verbose,
                    TEXT("Failed to chmod file '%s', got result code %d."),
                    *(DestinationPath / ZipFileName),
                    ChmodResult);
            }
#endif
        }
        else
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Verbose,
                TEXT("ZIP index %d: name='%s', opsys=%u (other), attr=%u."),
                i,
                *ZipFileName,
                OpSys,
                Attr);
            ExtractFile(ZipFile, i, ZipFileStat, ZipFileName, DestinationPath);
        }
    }

    // @note: This is required to keep these values alive until this point.
    ZipFileCleanup.Destroy();
    ZipErrorCleanup.Destroy();

    // @note: We no longer automatically fix up broken macOS builds of the Developer Authentication Tool, where the
    // Electron Framework isn't packaged properly. These broken builds were last around in the 1.13.x-ish series and
    // shouldn't be present in any modern version of the EOS SDK.

#if PLATFORM_MAC
    // @todo: Is this necessary now that we do chmod() on macOS based on the files as they were packaged in the ZIP
    // originally?
    UE_LOG(
        LogRedpointEOSEditor,
        Verbose,
        TEXT("Updating executable bits on the files extracted for the Developer Authentication Tool."));
    int32 ReturnCode;
    FString Stdout;
    FString Stderr;
    FPlatformProcess::ExecProcess(
        TEXT("/bin/chmod"),
        TEXT("-v -R a+rwx EOS_DevAuthTool.app"),
        &ReturnCode,
        &Stdout,
        &Stderr,
        *Path);
    if (ReturnCode != 0)
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Error,
            TEXT("Failed to set the executable bits on the files extracted from the Developer Authentication Tool; "
                 "stdout '%s', stderr '%s'."),
            *Stdout,
            *Stderr);
        return false;
    }
#endif

    return true;
}

bool FDevAuthToolLauncher::IsDevAuthToolRunning()
{
    // If the project is configured to use a remote host (not localhost) as the developer
    // tool address, then we assume it is running on the remote machine.
    // NOTE: We could try to do full host:port parsing here, but it's complicated
    // with IPv6 and Unreal doesn't have any good built-in utilities to do it.
    if (!MakeShared<Redpoint::EOS::Config::FIniConfig>()->GetDeveloperToolAddress().StartsWith("localhost:"))
    {
        return true;
    }

    if (!bNeedsToCheckIfDevToolRunning)
    {
        return bIsDevToolRunning;
    }

    this->ScheduleClearKnowledgeThatDevAuthToolIsRunning();

#if PLATFORM_WINDOWS
    auto *DevAuthToolProcessName = TEXT("EOS_DevAuthTool.exe");
#else
    auto *DevAuthToolProcessName = TEXT("EOS_DevAuthTool");
#endif

    UE_LOG(LogRedpointEOSEditor, Verbose, TEXT("Checking if %s process is running"), DevAuthToolProcessName);
    bIsDevToolRunning = FPlatformProcess::IsApplicationRunning(DevAuthToolProcessName);
    bNeedsToCheckIfDevToolRunning = false;

    return bIsDevToolRunning;
}

bool FDevAuthToolLauncher::Launch(bool bInteractive, bool bForceReinstall)
{
    using namespace ::Redpoint::EOS::Core::Editor;

    // Check to see if we can locate the EOS SDK Tools folder first.
    FString Path = FDevAuthToolLauncher::GetPathToToolsFolder();
    if (Path.IsEmpty())
    {
        if (bInteractive)
        {
            FText Title = LOCTEXT("DevToolPromptMissingToolsFolderTitle", "Unable to locate the EOS SDK Tools folder");
            FMessageDialog::Open(
                EAppMsgCategory::Error,
                EAppMsgType::Ok,
                LOCTEXT(
                    "DevToolPromptMissingToolsFolder",
                    "Unable to locate the path to the EOS SDK Tools folder; the Developer Authentication Tool can not "
                    "be launched."),
#if REDPOINT_EOS_UE_5_3_OR_LATER
                Title
#else
                &Title
#endif
            );
        }
        else
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Error,
                TEXT("Unable to locate the path to the EOS SDK Tools folder; the Developer Authentication Tool can not "
                     "be launched."));
        }
        return true;
    }

    // Calculate the executable path and paths that must exist in order for the Developer Authentication Tool to
    // launch.
#if PLATFORM_WINDOWS
    FString ExePath = Path / TEXT("DevAuthTool") / TEXT("EOS_DevAuthTool.exe");
    FString ExeDirPath = Path / TEXT("DevAuthTool");
    auto RequiredPaths = TArray<FString>{ExePath};
    FString StartPath = ExePath;
    FString StartArguments = TEXT("--port=6300");
#else
    FString ExePath = Path / TEXT("EOS_DevAuthTool.app") / TEXT("Contents") / TEXT("MacOS") / TEXT("EOS_DevAuthTool");
    FString ExeDirPath = Path;
    FString LinkPath = Path / TEXT("EOS_DevAuthTool.app") / TEXT("Contents") / TEXT("Frameworks") /
                       TEXT("Electron Framework.framework") / TEXT("Electron Framework");
    auto RequiredPaths = TArray<FString>{ExePath, LinkPath};
    FString StartPath = TEXT("/usr/bin/open");
    FString StartArguments = TEXT("EOS_DevAuthTool.app --args --port=6300");
#endif

    // If we don't have all the required paths, perform extraction.
    bool bAllPathsExist = RequiredPaths.Num() > 0;
    if (bForceReinstall)
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Verbose,
            TEXT("%s: We will forcibly re-extract the Developer Authentication Tool because bForceReinstall is set to "
                 "true."),
            ANSI_TO_TCHAR(__FUNCTION__));
        bAllPathsExist = false;
    }
    for (const auto &CheckPath : RequiredPaths)
    {
        if (!FPaths::FileExists(CheckPath))
        {
            bAllPathsExist = false;
            break;
        }
    }
    if (!bAllPathsExist)
    {
        if (!ExtractDeveloperAuthenticationTool(Path, bInteractive))
        {
            return false;
        }
    }

    // Start the Developer Authentication Tool.
    if (bInteractive)
    {
        FText StartTitle = LOCTEXT("DevToolPromptTitle", "How to use the Developer Authentication Tool");
        FMessageDialog::Open(
            EAppMsgType::Ok,
            LOCTEXT(
                "DevToolPrompt",
                "If prompted for a port, use port 6300.\n\n"
                "Name each credential Context_1, Context_2, etc.\n\n"
                "Do not use the same Epic Games account more than once.\n\n"
                "For more information, check the documentation."),
#if REDPOINT_EOS_UE_5_3_OR_LATER
            StartTitle
#else
            &StartTitle
#endif
        );
    }

    UE_LOG(LogRedpointEOSEditor, Verbose, TEXT("Launching Developer Authentication Tool located at: %s"), *ExePath);

#if PLATFORM_WINDOWS
    if (StartPath.Len() >= WINDOWS_MAX_PATH)
    {
        UE_LOG(
            LogRedpointEOSEditor,
            Error,
            TEXT("Path limit exceeded for Windows while launching Developer Authentication Tool located at: %s"),
            *Path);

        // Start the Developer Authentication Tool.
        if (bInteractive)
        {
            FText TooLongTitle = LOCTEXT("DevToolTooLongPromptTitle", "Path to EOS SDK is too long");
            if (FMessageDialog::Open(
                    EAppMsgType::YesNo,
                    LOCTEXT(
                        "DevToolTooLongPromptBody",
                        "The EOS SDK is installed in a path that is too long for the Developer Authentication Tool to "
                        "be started. Please move it to a shorter location.\n\nWould you like to open the "
                        "documentation?"),
#if REDPOINT_EOS_UE_5_3_OR_LATER
                    TooLongTitle
#else
                    &TooLongTitle
#endif
                    ) == EAppReturnType::Yes)
            {
                FPlatformProcess::LaunchURL(
                    TEXT("https://docs.redpoint.games/eos-online-subsystem/docs/#installing-the-eos-sdk"),
                    nullptr,
                    nullptr);
            }
        }
    }
#endif

    uint32 ProcID;
    FPlatformProcess::CreateProc(
        *StartPath,
        *StartArguments,
        true,
        false,
        false,
        &ProcID,
        0,
        *ExeDirPath,
        nullptr,
        nullptr);
    this->ScheduleClearKnowledgeThatDevAuthToolIsRunning();
    bIsDevToolRunning = true;
    bNeedsToCheckIfDevToolRunning = false;

    return true;
}

}

#undef LOCTEXT_NAMESPACE

#endif

REDPOINT_EOS_CODE_GUARD_END()