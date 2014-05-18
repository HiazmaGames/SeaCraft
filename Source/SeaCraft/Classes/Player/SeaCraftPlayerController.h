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

	/** Initialize the input system from the player settings */
	virtual void InitInputSystem() OVERRIDE;

	/** Check if gameplay related actions (movement, weapon usage, etc) are allowed right now */
	bool IsGameInputAllowed() const;

	/** Local function to change system settings and apply them */
	UFUNCTION(exec, BlueprintCallable, Category = "Settings")
	void SetGraphicsQuality(int32 InGraphicsQuality);

	/** Fast game shutdown */
	UFUNCTION(exec)
	void Quit();

protected:
	/** If set, gameplay related actions (movement, weapon usage, etc) are allowed */
	uint8 bAllowGameActions : 1;
};
