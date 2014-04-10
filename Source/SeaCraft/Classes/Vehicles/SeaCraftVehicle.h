// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "GameFramework/Pawn.h"
#include "SeaCraftVehicle.generated.h"

/**
 * 
 */
UCLASS(abstract, HeaderGroup=Pawn, config = Game, dependson = AController, BlueprintType, Blueprintable, hidecategories = (Navigation, "AI|Navigation"))
class ASeaCraftVehicle : public APawn
{
	GENERATED_UCLASS_BODY()

	
};
