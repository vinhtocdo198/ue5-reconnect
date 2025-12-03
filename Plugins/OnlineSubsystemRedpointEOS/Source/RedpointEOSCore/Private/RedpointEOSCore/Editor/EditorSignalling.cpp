// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Editor/EditorSignalling.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(683639614, Redpoint::EOS::Core::Editor)
{

#if WITH_EDITOR

FEditorSignalling::FOnEditorCustomSignal FEditorSignalling::OnEditorCustomSignalValue;

FEditorSignalling::FOnEditorCustomSignal &FEditorSignalling::OnEditorCustomSignal()
{
    return FEditorSignalling::OnEditorCustomSignalValue;
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()