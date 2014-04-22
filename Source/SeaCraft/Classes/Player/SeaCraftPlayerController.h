// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "SeaCraftPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ASeaCraftPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

	/** Check if gameplay related actions (movement, weapon usage, etc) are allowed right now */
	bool IsGameInputAllowed() const;

protected:
	/** If set, gameplay related actions (movement, weapon usage, etc) are allowed */
	uint8 bAllowGameActions : 1;
};
