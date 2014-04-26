// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

FName AShipVehicle::VehicleMovementComponentName(TEXT("ShipMovementComp"));

AShipVehicle::AShipVehicle(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	CameraSpringArm = PCIP.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("ShipCameraSpringArm"));
	CameraSpringArm->AttachTo(VehicleMesh);

	VehicleCamera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("ShipVehicleCamera"));
	VehicleCamera->AttachTo(CameraSpringArm);

	VehicleMovement = PCIP.CreateDefaultSubobject<UShipVehicleMovementComponent>(this, VehicleMovementComponentName);
	VehicleMovement->SetIsReplicated(true); // Enable replication by default
	VehicleMovement->UpdatedComponent = VehicleMesh;
}
