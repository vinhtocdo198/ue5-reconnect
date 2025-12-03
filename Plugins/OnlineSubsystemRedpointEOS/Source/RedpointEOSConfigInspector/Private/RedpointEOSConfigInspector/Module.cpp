// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSConfigInspector/Module.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER

#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STreeView.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define LOCTEXT_NAMESPACE "RedpointEOSConfigInspector"

static const FName ConfigInspectorTabName = FName(TEXT("ConfigInspector"));

void FModule::StartupModule()
{
    FGlobalTabmanager::Get()
        ->RegisterNomadTabSpawner(
            ConfigInspectorTabName,
            FOnSpawnTab::CreateRaw(this, &FModule::CreateConfigInspectorTab))
        .SetDisplayName(LOCTEXT("ConfigInspectorTabTitle", "Config Inspector"))
        .SetTooltipText(LOCTEXT("ConfigInspectorTabToolTipText", "Config Inspector"))
        .SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());
}

void FModule::ShutdownModule()
{
    if (FSlateApplication::IsInitialized())
    {
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ConfigInspectorTabName);
    }
}

TSharedRef<SDockTab> FModule::CreateConfigInspectorTab(const FSpawnTabArgs &Args)
{
    return SAssignNew(ConfigInspectorTab, SDockTab)
        .TabRole(ETabRole::NomadTab)[CreateConfigInspectorDialog().ToSharedRef()];
}

enum EConfigEntryType
{
    Branch,
    Layer,
    Section,
    Value,
};

enum EConfigLayerType
{
    Static,
    Dynamic,
    Saved,
    CombinedStatic,
    FinalCombined,
    CommandLineOverrides,
    InMemory,
    RuntimeChanges,
};

class FConfigEntry
{
public:
    FConfigEntry()
        : Children()
        , EntryType()
        , BranchName()
        , Branch()
        , LayerType()
        , StaticLayerName()
        , Section()
        , Key()
        , Value()
    {
    }

    FConfigEntry(const EConfigEntryType &InEntryType, const FConfigEntry &InExisting)
        : Children()
        , EntryType(InEntryType)
        , BranchName(InExisting.BranchName)
        , Branch(InExisting.Branch)
        , LayerType(InExisting.LayerType)
        , StaticLayerName(InExisting.StaticLayerName)
        , Section(InExisting.Section)
        , Key(InExisting.Key)
        , Value(InExisting.Value)
    {
    }

    TArray<TSharedPtr<FConfigEntry>> Children;
    EConfigEntryType EntryType;
    FName BranchName;
    FConfigBranch *Branch;
    EConfigLayerType LayerType;
    FString StaticLayerName;
    FName FileName;
    FString FilePlatformName;
    FName FileTag;
    FString Section;
    FName Key;
    FConfigValue Value;
};

static TArray<TSharedPtr<FName>> PlatformArray{
    MakeShared<FName>(NAME_None),
    MakeShared<FName>("Windows"),
    MakeShared<FName>("Mac"),
    MakeShared<FName>("Linux"),
    MakeShared<FName>("Android"),
    MakeShared<FName>("IOS")};

TSharedPtr<SWidget> FModule::CreateConfigInspectorDialog()
{
    auto ConfigObservableArray = MakeShared<UE::Slate::Containers::TObservableArray<TSharedPtr<FConfigEntry>>>();
    PopulateObservableArray(ConfigObservableArray, NAME_None);

    auto PlatformSelectorText = SNew(STextBlock).MinDesiredWidth(50.0f).Text(FText::FromString(TEXT("None")));
    auto PlatformSelector =
        SNew(SComboBox<TSharedPtr<FName>>)
            .OptionsSource(&PlatformArray)
            .InitiallySelectedItem(PlatformArray[0])
            .OnGenerateWidget_Lambda([](TSharedPtr<FName> Value) {
                return SNew(STextBlock).Text(FText::FromString(Value->ToString()));
            })
            .OnSelectionChanged_Lambda(
                [ConfigObservableArray, PlatformSelectorText](TSharedPtr<FName> SelectedValue, ESelectInfo::Type) {
                    auto PlatformName = SelectedValue.IsValid() ? *SelectedValue : NAME_None;
                    PlatformSelectorText->SetText(FText::FromString(PlatformName.ToString()));
                    PopulateObservableArray(ConfigObservableArray, PlatformName);
                })[PlatformSelectorText];

    auto RefreshButton = SNew(SButton)[SNew(STextBlock).Text(LOCTEXT("Refresh", "Refresh"))].OnPressed_Lambda(
        [ConfigObservableArray, PlatformSelector]() {
            PopulateObservableArray(ConfigObservableArray, *PlatformSelector->GetSelectedItem());
        });

    auto ConfigTree =
        SNew(STreeView<TSharedPtr<FConfigEntry>>)
            .TreeItemsSource(ConfigObservableArray)
            .OnGenerateRow_Lambda([](TSharedPtr<FConfigEntry> Value, const TSharedRef<STableViewBase> &OwnerTable) {
                typedef STableRow<TSharedPtr<FConfigEntry>> RowType;

                TSharedRef<RowType> NewRow = SNew(RowType, OwnerTable);
                if (Value != nullptr && Value->EntryType == EConfigEntryType::Value)
                {
                    auto ValueColumns = SNew(SHorizontalBox) +
                                        SHorizontalBox::Slot().FillWidth(0.25f)[SNew(STextBlock).Text_Lambda([Value]() {
                                            return FText::FromString(Value->Key.ToString());
                                        })] +
                                        SHorizontalBox::Slot().FillWidth(0.25f)[SNew(STextBlock).Text_Lambda([Value]() {
                                            FString ValueType;
                                            switch (Value->Value.ValueType)
                                            {
                                            case FConfigValue::EValueType::Set:
                                                ValueType = TEXT("Set =");
                                                break;
                                            case FConfigValue::EValueType::ArrayAdd:
                                                ValueType = TEXT("ArrayAdd .");
                                                break;
                                            case FConfigValue::EValueType::ArrayAddUnique:
                                                ValueType = TEXT("ArrayAddUnique +");
                                                break;
                                            case FConfigValue::EValueType::Remove:
                                                ValueType = TEXT("Remove -");
                                                break;
                                            case FConfigValue::EValueType::Clear:
                                                ValueType = TEXT("Clear !");
                                                break;
                                            case FConfigValue::EValueType::ArrayOfStructKey:
                                                ValueType = TEXT("ArrayOfStructKey @");
                                                break;
                                            case FConfigValue::EValueType::POCArrayOfStructKey:
                                                ValueType = TEXT("POCArrayOfStructKey *");
                                                break;
                                            case FConfigValue::EValueType::Combined:
                                                ValueType = TEXT("Combined");
                                                break;
                                            case FConfigValue::EValueType::ArrayCombined:
                                                ValueType = TEXT("ArrayCombined");
                                                break;
                                            default:
                                                ValueType = TEXT("Unknown");
                                                break;
                                            }
                                            return FText::FromString(ValueType);
                                        })] +
                                        SHorizontalBox::Slot().FillWidth(0.25f)[SNew(STextBlock).Text_Lambda([Value]() {
                                            return FText::FromString(Value->Value.GetValue());
                                        })] +
                                        SHorizontalBox::Slot().FillWidth(0.25f)[SNew(STextBlock).Text_Lambda([Value]() {
                                            return FText::FromString(Value->Value.GetSavedValue());
                                        })];

#if CONFIG_CAN_SAVE_COMMENTS
                    if (!Value->Value.Comment.IsEmpty())
                    {
                        auto Row = SNew(SVerticalBox) +
                                   SVerticalBox::Slot()[SNew(STextBlock)
                                                            .Text_Lambda([Value]() {
                                                                return FText::FromString(Value->Value.Comment);
                                                            })
                                                            .ColorAndOpacity(FSlateColor(FColor(127, 255, 127)))] +
                                   SVerticalBox::Slot()[ValueColumns];
                        NewRow->SetContent(Row);
                    }
                    else
                    {
#endif
                        NewRow->SetContent(ValueColumns);
#if CONFIG_CAN_SAVE_COMMENTS
                    }
#endif
                }
                else
                {
                    NewRow->SetContent(SNew(STextBlock).Text_Lambda([Value]() {
                        if (Value != nullptr)
                        {
                            switch (Value->EntryType)
                            {
                            case EConfigEntryType::Branch: {
                                FString ReplayMethodString;
                                switch (Value->Branch->ReplayMethod)
                                {
                                case EBranchReplayMethod::FullReplay:
                                    ReplayMethodString = TEXT("FullReplay");
                                    break;
                                case EBranchReplayMethod::DynamicLayerReplay:
                                    ReplayMethodString = TEXT("DynamicLayerReplay");
                                    break;
                                case EBranchReplayMethod::NoReplay:
                                    ReplayMethodString = TEXT("NoReplay");
                                    break;
                                default:
                                    ReplayMethodString = TEXT("Unknown");
                                    break;
                                }
                                return FText::Format(
                                    LOCTEXT("EntryBranch", "{0} ({1}, {2}, {3})"),
                                    FText::FromString(Value->BranchName.ToString()),
                                    FText::FromString(Value->Branch->Platform.ToString()),
                                    FText::FromString(ReplayMethodString),
                                    FText::FromString(Value->Branch->IniPath));
                            }
                            case EConfigEntryType::Layer:
                                switch (Value->LayerType)
                                {
                                case EConfigLayerType::Static:
                                    return FText::Format(
                                        LOCTEXT("LayerTypeStatic", "Static = {0}"),
                                        FText::FromString(Value->StaticLayerName));
                                case EConfigLayerType::Dynamic:
                                    return LOCTEXT("LayerTypeDynamic", "Dynamic");
                                case EConfigLayerType::Saved:
                                    return LOCTEXT("LayerTypeSaved", "Saved");
                                case EConfigLayerType::CombinedStatic:
                                    return FText::Format(
                                        LOCTEXT("LayerTypeCombinedStatic", "Combined Static = ({0}, {1}, {2})"),
                                        FText::FromString(Value->FileName.ToString()),
                                        FText::FromString(Value->FilePlatformName),
                                        FText::FromString(Value->FileTag.ToString()));
                                case EConfigLayerType::FinalCombined:
                                    return FText::Format(
                                        LOCTEXT("LayerTypeFinalCombined", "Final Combined = ({0}, {1}, {2})"),
                                        FText::FromString(Value->FileName.ToString()),
                                        FText::FromString(Value->FilePlatformName),
                                        FText::FromString(Value->FileTag.ToString()));
                                case EConfigLayerType::CommandLineOverrides:
                                    return LOCTEXT("LayerTypeCommandLineOverrides", "Command Line Overrides");
                                case EConfigLayerType::InMemory:
                                    return FText::Format(
                                        LOCTEXT("LayerTypeInMemory", "In Memory = ({0}, {1}, {2})"),
                                        FText::FromString(Value->FileName.ToString()),
                                        FText::FromString(Value->FilePlatformName),
                                        FText::FromString(Value->FileTag.ToString()));
                                case EConfigLayerType::RuntimeChanges:
                                    return LOCTEXT("LayerTypeRuntimeChanges", "Runtime Changes");
                                default:
                                    return LOCTEXT("LayerTypeUnknown", "Unknown");
                                }
                            case EConfigEntryType::Section:
                                return FText::FromString(Value->Section);
                            case EConfigEntryType::Value:
                                checkf(false, TEXT("Expected value entries to be handled with columns!"));
                                return LOCTEXT("EntryTypeUnhandled", "Unhandled");
                            default:
                                return LOCTEXT("EntryTypeUnknown", "Unknown");
                            }
                        }
                        return LOCTEXT("EntryNull", "Null");
                    }));
                }

                return NewRow;
            })
            .OnGetChildren_Lambda([](TSharedPtr<FConfigEntry> Parent, TArray<TSharedPtr<FConfigEntry>> &OutChildren) {
                OutChildren = Parent->Children;
            });

    return SNew(SVerticalBox) +
           SVerticalBox::Slot().AutoHeight()
               [SNew(SHorizontalBox) + SHorizontalBox::Slot().Padding(FMargin(5.0f)).AutoWidth()[PlatformSelector] +
                SHorizontalBox::Slot().Padding(FMargin(5.0f)).AutoWidth()[RefreshButton]] +
           SVerticalBox::Slot().Padding(FMargin(5.0f)).FillHeight(1.0f)[ConfigTree];
}

void FModule::PopulateObservableArray(
    const TSharedRef<UE::Slate::Containers::TObservableArray<TSharedPtr<FConfigEntry>>> &ConfigObservableArray,
    FName PlatformName)
{
    ConfigObservableArray->Reset(0);

#define REDPOINT_EOS_INI_FILE_AS_NAME(IniName) FName(#IniName),
    TArray<FName> KnownInis{ENUMERATE_KNOWN_INI_FILES(REDPOINT_EOS_INI_FILE_AS_NAME)};
#undef REDPOINT_EOS_INI_FILE_AS_NAME

    auto GenerateEntriesForSection = [](TSharedRef<FConfigEntry> SectionEntry, const FConfigSectionMap &SectionMap) {
        for (const auto &ValueKV : SectionMap)
        {
            auto ValueEntry = MakeShared<FConfigEntry>(EConfigEntryType::Value, *SectionEntry);
            ValueEntry->Key = ValueKV.Key;
            ValueEntry->Value = ValueKV.Value;
            SectionEntry->Children.Add(ValueEntry);
        }
    };
    auto GenerateEntriesForCommandStream = [=](TSharedRef<FConfigEntry> LayerEntry,
                                               const FConfigCommandStream &CommandStream) {
        for (const auto &SectionKV : CommandStream)
        {
            auto SectionEntry = MakeShared<FConfigEntry>(EConfigEntryType::Section, *LayerEntry);
            SectionEntry->Section = SectionKV.Key;
            GenerateEntriesForSection(SectionEntry, SectionKV.Value);
            LayerEntry->Children.Add(SectionEntry);
        }
    };
    auto GenerateEntriesForConfigFile = [=](TSharedRef<FConfigEntry> LayerEntry, const FConfigFile &ConfigFile) {
        LayerEntry->FileName = ConfigFile.Name;
        LayerEntry->FilePlatformName = ConfigFile.PlatformName;
        LayerEntry->FileTag = ConfigFile.Tag;
        for (const auto &SectionKV : ConfigFile)
        {
            auto SectionEntry = MakeShared<FConfigEntry>(EConfigEntryType::Section, *LayerEntry);
            SectionEntry->Section = SectionKV.Key;
            GenerateEntriesForSection(SectionEntry, SectionKV.Value);
            LayerEntry->Children.Add(SectionEntry);
        }
    };

    FConfigCacheIni *Cache = PlatformName.IsNone() ? GConfig : GConfig->ForPlatform(PlatformName);

    for (const auto &KnownIni : KnownInis)
    {
        auto BranchEntry = MakeShared<FConfigEntry>();
        BranchEntry->EntryType = EConfigEntryType::Branch;
        BranchEntry->BranchName = KnownIni;
        BranchEntry->Branch = Cache->FindBranch(KnownIni, TEXT(""));

        for (const auto &StaticLayerKV : BranchEntry->Branch->StaticLayers)
        {
            auto LayerEntry = MakeShared<FConfigEntry>(EConfigEntryType::Layer, *BranchEntry);
            LayerEntry->LayerType = EConfigLayerType::Static;
            LayerEntry->StaticLayerName = StaticLayerKV.Key;
            GenerateEntriesForCommandStream(LayerEntry, StaticLayerKV.Value);
            BranchEntry->Children.Add(LayerEntry);
        }
        for (const auto &DynamicLayer : BranchEntry->Branch->DynamicLayers)
        {
            auto LayerEntry = MakeShared<FConfigEntry>(EConfigEntryType::Layer, *BranchEntry);
            LayerEntry->LayerType = EConfigLayerType::Dynamic;
            GenerateEntriesForCommandStream(LayerEntry, *DynamicLayer);
            BranchEntry->Children.Add(LayerEntry);
        }
        {
            auto LayerEntry = MakeShared<FConfigEntry>(EConfigEntryType::Layer, *BranchEntry);
            LayerEntry->LayerType = EConfigLayerType::Saved;
            GenerateEntriesForCommandStream(LayerEntry, BranchEntry->Branch->SavedLayer);
            BranchEntry->Children.Add(LayerEntry);
        }
        {
            auto LayerEntry = MakeShared<FConfigEntry>(EConfigEntryType::Layer, *BranchEntry);
            LayerEntry->LayerType = EConfigLayerType::CombinedStatic;
            GenerateEntriesForConfigFile(LayerEntry, BranchEntry->Branch->CombinedStaticLayers);
            BranchEntry->Children.Add(LayerEntry);
        }
        {
            auto LayerEntry = MakeShared<FConfigEntry>(EConfigEntryType::Layer, *BranchEntry);
            LayerEntry->LayerType = EConfigLayerType::FinalCombined;
            GenerateEntriesForConfigFile(LayerEntry, BranchEntry->Branch->FinalCombinedLayers);
            BranchEntry->Children.Add(LayerEntry);
        }
        {
            auto LayerEntry = MakeShared<FConfigEntry>(EConfigEntryType::Layer, *BranchEntry);
            LayerEntry->LayerType = EConfigLayerType::CommandLineOverrides;
            GenerateEntriesForCommandStream(LayerEntry, BranchEntry->Branch->CommandLineOverrides);
            BranchEntry->Children.Add(LayerEntry);
        }
        {
            auto LayerEntry = MakeShared<FConfigEntry>(EConfigEntryType::Layer, *BranchEntry);
            LayerEntry->LayerType = EConfigLayerType::InMemory;
            GenerateEntriesForConfigFile(LayerEntry, BranchEntry->Branch->InMemoryFile);
            BranchEntry->Children.Add(LayerEntry);
        }
        {
            auto LayerEntry = MakeShared<FConfigEntry>(EConfigEntryType::Layer, *BranchEntry);
            LayerEntry->LayerType = EConfigLayerType::RuntimeChanges;
            GenerateEntriesForCommandStream(LayerEntry, BranchEntry->Branch->RuntimeChanges);
            BranchEntry->Children.Add(LayerEntry);
        }

        ConfigObservableArray->Add(BranchEntry);
    }
}

#undef LOCTEXT_NAMESPACE

REDPOINT_EOS_CODE_GUARD_END()

#endif

IMPLEMENT_MODULE(FModule, RedpointEOSConfigInspector);