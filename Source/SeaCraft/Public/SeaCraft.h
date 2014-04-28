// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "EngineKismetLibraryClasses.h"
#include "ParticleDefinitions.h"
#include "Net/UnrealNetwork.h"
#include "SeaCraftClasses.h"
#include "SeaCraftComponentClasses.h"
#include "SeaCraftPawnClasses.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShipPhysics, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogOcean, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWeapon, Log, All);

/** When you modify this, please note that this information can be saved with instances
* also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
#define COLLISION_WEAPON		ECC_GameTraceChannel1
#define COLLISION_PROJECTILE	ECC_GameTraceChannel2
