
// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointOSSv1PartyRequestToJoinRemovedReason.generated.h"

UENUM(BlueprintType, Meta = (DisplayName = "Party Request To Join Removed Reason"))
enum class ERedpointOSSv1PartyRequestToJoinRemovedReason : uint8
{
	/** Unknown or undefined reason */
	Unknown,

	/** Cancelled */
	Cancelled,

	/** Expired */
	Expired,

	/** Dismissed */
	Dismissed,

	/** Accepted */
	Accepted,
};