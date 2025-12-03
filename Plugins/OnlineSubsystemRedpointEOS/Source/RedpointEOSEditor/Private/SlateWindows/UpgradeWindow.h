// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#if REDPOINT_EOS_IS_FREE_EDITION

#include "Input/Reply.h"
#include "Widgets/SWindow.h"

class FUpgradeWindow
{
private:
    TSharedPtr<SWindow> UpgradePromptWindow;

    FReply FollowUpgradePrompt();

public:
    FUpgradeWindow()
        : UpgradePromptWindow(nullptr){};
    bool Open();
};

#endif