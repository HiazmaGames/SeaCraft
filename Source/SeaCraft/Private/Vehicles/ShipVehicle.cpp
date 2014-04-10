// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

FName AShipVehicle::VehicleMovementComponentName(TEXT("ShipMovementComp"));
FName AShipVehicle::VehicleMeshComponentName(TEXT("ShipVehicleMesh"));

AShipVehicle::AShipVehicle(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	VehicleMesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, VehicleMeshComponentName);
	static FName CollisionProfileName(TEXT("Vehicle"));
	VehicleMesh->SetCollisionProfileName(CollisionProfileName);
	VehicleMesh->BodyInstance.bSimulatePhysics = true;
	VehicleMesh->BodyInstance.bNotifyRigidBodyCollision = true;
	VehicleMesh->BodyInstance.bUseCCD = true;
	VehicleMesh->bBlendPhysics = true;
	VehicleMesh->bUseSingleBodyPhysics = true;
	VehicleMesh->bGenerateOverlapEvents = true;
	RootComponent = VehicleMesh;

	VehicleMovement = PCIP.CreateDefaultSubobject<UShipVehicleMovementComponent>(this, VehicleMovementComponentName);
	VehicleMovement->SetIsReplicated(true); // Enable replication by default
	VehicleMovement->UpdatedComponent = VehicleMesh;
}
