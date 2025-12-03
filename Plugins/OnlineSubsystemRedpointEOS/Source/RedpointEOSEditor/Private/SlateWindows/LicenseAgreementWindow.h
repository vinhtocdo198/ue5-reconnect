// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#if REDPOINT_EOS_IS_FREE_EDITION

#include "Input/Reply.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

class FLicenseAgreementWindow
{
private:
    TSharedPtr<SWindow> LicenseAgreementWindow;
    TSharedPtr<STextBlock> LicenseAgreementTextBox;
    bool LicenseAgreementLoaded;

    FReply ViewLicenseTermsOnWebsite();
    FReply AcceptLicenseTerms();
    FReply RejectLicenseTerms();

public:
    FLicenseAgreementWindow()
        : LicenseAgreementWindow(nullptr)
        , LicenseAgreementTextBox(nullptr)
        , LicenseAgreementLoaded(false){};
    bool Open();
};

#endif