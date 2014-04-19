// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#pragma once

#include "GameFramework/Pawn.h"
#include "SeaCraftVehicle.generated.h"

namespace EVWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		PreFire,
		PostFire
	};
}

/**	The FVWeaponData struct defines each available weapon in the vehicle */
USTRUCT()
struct FVWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** Vehicle-unique ID to get and/or reference weapon in blueprints */
	UPROPERTY(EditDefaultsOnly, Category = WeaponInfo)
	FString WeaponID;

	/** Vehicle-unique group ID to control weapons in group simultaneously */
	UPROPERTY(EditDefaultsOnly, Category = WeaponInfo)
	FString GroupID;

	/** Firing sockets names */
	UPROPERTY(EditDefaultsOnly, Category = Turrets)
	TArray<FName> TurretSockets;

	/** Defaults */
	FVWeaponData()
	{
		WeaponID = TEXT("MustBeVehicleUnique");
		GroupID = TEXT("Default");
	}
};

/**
 * 
 */
UCLASS(abstract, HeaderGroup=Pawn, config = Game, dependson = AController, BlueprintType, Blueprintable, hidecategories = (Navigation, "AI|Navigation"))
class ASeaCraftVehicle : public APawn
{
	GENERATED_UCLASS_BODY()

	/** Information for each weapon a player may control */
	UPROPERTY(Category = Vehicle, EditDefaultsOnly, BlueprintReadOnly)
	TArray<FVWeaponData> VehicleWeapons;
	
};
