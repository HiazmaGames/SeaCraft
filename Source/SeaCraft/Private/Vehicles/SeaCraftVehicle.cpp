// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

ASeaCraftVehicle::ASeaCraftVehicle(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bWantsToFire = false;
}

void ASeaCraftVehicle::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// @TODO List components, find first group, and make it active
}


//////////////////////////////////////////////////////////////////////////
// Weapon control

void ASeaCraftVehicle::SetWeaponGroup(FName WeaponGroup)
{
	CurrentWeaponGroup = WeaponGroup;
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

void ASeaCraftVehicle::StartWeaponFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		
		// @TODO Current weapons start fire
	}
}

void ASeaCraftVehicle::StopWeaponFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		
		// @TODO Current weapons stop fire
	}
}

bool ASeaCraftVehicle::CanFire() const
{
	// @TODO Check vehicle health, weapon distance and state, obstacles, etc.
	return true;
}


//////////////////////////////////////////////////////////////////////////
// Input

void ASeaCraftVehicle::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	InputComponent->BindAction("StartFire", IE_Pressed, this, &ASeaCraftVehicle::OnStartFire);
	InputComponent->BindAction("StopFire", IE_Released, this, &ASeaCraftVehicle::OnStopFire);

	InputComponent->BindAction("NextWeapon", IE_Pressed, this, &ASeaCraftVehicle::OnNextWeapon);
	InputComponent->BindAction("PrevWeapon", IE_Pressed, this, &ASeaCraftVehicle::OnPrevWeapon);
}

void ASeaCraftVehicle::OnStartFire()
{
	ASeaCraftPlayerController* MyPC = Cast<ASeaCraftPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		StartWeaponFire();
	}
}

void ASeaCraftVehicle::OnStopFire()
{
	StopWeaponFire();
}

void ASeaCraftVehicle::OnNextWeapon()
{
	// @TODO OnNextWeapon
}

void ASeaCraftVehicle::OnPrevWeapon()
{
	// @TODO OnPrevWeapon
}
