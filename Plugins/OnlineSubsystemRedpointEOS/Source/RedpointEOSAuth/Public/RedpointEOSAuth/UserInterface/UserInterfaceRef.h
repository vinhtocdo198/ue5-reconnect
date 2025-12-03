// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"
#include "UObject/UObjectGlobals.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3131108048, Redpoint::EOS::Auth::UserInterface)
{
using namespace ::Redpoint::EOS::Auth;

template <typename TWidgetInterfaceType, typename TWidgetInterfaceUClassType, typename TWidgetContextType>
class TUserInterfaceRef
    : public TSharedFromThis<TUserInterfaceRef<TWidgetInterfaceType, TWidgetInterfaceUClassType, TWidgetContextType>>
{
private:
    TSoftObjectPtr<UUserWidget> Widget;
    TSoftObjectPtr<TWidgetContextType> Context;
    bool bIsValid;
    FString InvalidErrorMessage;

public:
    TUserInterfaceRef(
        const TSharedRef<FAuthenticationGraphState> &State,
        FString InWidgetAssetPath,
        const FString &InRequiredInterfaceName)
        : Widget(nullptr)
        , Context(nullptr)
        , bIsValid(false)
        , InvalidErrorMessage()
    {
        // Automatically redirect widget paths from our old UMG widgets (that no longer exist) to our Slate widgets.
        if (InWidgetAssetPath ==
            "/OnlineSubsystemRedpointEOS/"
            "EOSDefaultUserInterface_EnterDevicePinCode.EOSDefaultUserInterface_EnterDevicePinCode_C")
        {
            InWidgetAssetPath = "/Script/RedpointEOSAuth.EOSDefaultUserInterface_EnterDevicePinCode";
        }
        if (InWidgetAssetPath ==
            "/OnlineSubsystemRedpointEOS/"
            "EOSDefaultUserInterface_SignInOrCreateAccount.EOSDefaultUserInterface_SignInOrCreateAccount_C")
        {
            InWidgetAssetPath = "/Script/RedpointEOSAuth.EOSDefaultUserInterface_SignInOrCreateAccount";
        }

        FSoftClassPath WidgetClassPath = FSoftClassPath(InWidgetAssetPath);
        TSubclassOf<UUserWidget> WidgetClass = WidgetClassPath.TryLoadClass<UUserWidget>();
        if (WidgetClass == nullptr)
        {
            this->InvalidErrorMessage = FString::Printf(
                TEXT("TUserInterfaceRef: Unable to locate the UMG widget at %s. Check that it exists and if "
                     "necessary, add it to your Asset Manager configuration so it gets included in packaged builds."),
                *WidgetClassPath.GetAssetPathString());
            return;
        }
        if (!WidgetClass->ImplementsInterface(TWidgetInterfaceUClassType::StaticClass()))
        {
            this->InvalidErrorMessage = FString::Printf(
                TEXT("TUserInterfaceRef: The configured UMG widget does not implement the "
                     "%s interface."),
                *InRequiredInterfaceName);
            return;
        }

        UWorld *WorldRef = State->GetWorld();
        if (WorldRef == nullptr)
        {
            this->InvalidErrorMessage =
                FString::Printf(TEXT("TUserInterfaceRef: There is no active world. A world is required in order to be "
                                     "able to create and display UMG widgets."));
            return;
        }

        this->Widget = NewObject<UUserWidget>(WorldRef->GetGameInstance(), WidgetClass, NAME_None, RF_Transactional);
        if (this->Widget == nullptr)
        {
            this->InvalidErrorMessage =
                FString::Printf(TEXT("TUserInterfaceRef: Unable to create UMG widget."), *InRequiredInterfaceName);
            return;
        }

        this->Context = NewObject<TWidgetContextType>(this->Widget.Get());

        this->bIsValid = true;
    }

    UE_NONCOPYABLE(TUserInterfaceRef);

    ~TUserInterfaceRef()
    {
        if (this->Widget.IsValid())
        {
            this->Widget->RemoveFromParent();
            this->Widget.Reset();
        }
        if (this->Context.IsValid())
        {
            this->Context.Reset();
        }

        this->bIsValid = false;
    }

    bool IsValid()
    {
        return this->bIsValid && this->Context.IsValid() && this->Widget.IsValid();
    }

    FString GetInvalidErrorMessage()
    {
        return this->InvalidErrorMessage;
    }

    void AddToState(const TSharedRef<FAuthenticationGraphState> &State)
    {
        check(this->IsValid());
        State->SetCurrentUserInterfaceWidget(this->Widget.Get());
    }

    void RemoveFromState(const TSharedRef<FAuthenticationGraphState> &State)
    {
        check(this->IsValid());

        // @todo: Validate that it is our widget that is set.
        State->ClearCurrentUserInterfaceWidget();
    }

    TWidgetContextType *GetContext()
    {
        check(this->IsValid());
        return this->Context.Get();
    }

    UUserWidget *GetWidget()
    {
        check(this->IsValid());
        return this->Widget.Get();
    }
};

}

namespace Redpoint::EOS::Auth::UserInterface
{
REDPOINT_EOS_FILE_NS_EXPORT(3131108048, Redpoint::EOS::Auth::UserInterface, TUserInterfaceRef)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()