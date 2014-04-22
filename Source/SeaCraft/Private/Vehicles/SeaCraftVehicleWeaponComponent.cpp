// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

USeaCraftVehicleWeaponComponent::USeaCraftVehicleWeaponComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bWantsToFire = false;
	CurrentState = EVWeaponState::Idle;
}

//////////////////////////////////////////////////////////////////////////
// Input

void USeaCraftVehicleWeaponComponent::StartFire()
{
	AActor* Owner = GetOwner();
	if (Owner == NULL)
	{
		return;
	}

	if (Owner->Role < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void USeaCraftVehicleWeaponComponent::StopFire()
{
	AActor* Owner = GetOwner();
	if (Owner == NULL)
	{
		return;
	}

	if (Owner->Role < ROLE_Authority)
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

bool USeaCraftVehicleWeaponComponent::ServerStartFire_Validate()
{
	return true;
}

void USeaCraftVehicleWeaponComponent::ServerStartFire_Implementation()
{
	StartFire();
}

bool USeaCraftVehicleWeaponComponent::ServerStopFire_Validate()
{
	return true;
}

void USeaCraftVehicleWeaponComponent::ServerStopFire_Implementation()
{
	StopFire();
}

//////////////////////////////////////////////////////////////////////////
// Control

bool USeaCraftVehicleWeaponComponent::CanFire() const
{
	// @TODO Check fire conditions
	// 1. check we have alive vehicle
	// 2. check we're idle or firing (else we can't fire)
	// 3. check we have no pending reload

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Weapon usage

void USeaCraftVehicleWeaponComponent::HandleFiring()
{
	// @TODO Make a shot, consume ammo, etc.
}

bool USeaCraftVehicleWeaponComponent::ServerHandleFiring_Validate()
{
	return true;
}

void USeaCraftVehicleWeaponComponent::ServerHandleFiring_Implementation()
{
	HandleFiring();

	// @TODO Check we should update ammo
}

void USeaCraftVehicleWeaponComponent::SetWeaponState(EVWeaponState::Type NewState)
{
	CurrentState = NewState;
}

void USeaCraftVehicleWeaponComponent::DetermineWeaponState()
{
	EVWeaponState::Type NewState = EVWeaponState::Idle;

	if ((bWantsToFire == true) && (CanFire() == true))
	{
		NewState = EVWeaponState::Firing;
	}

	SetWeaponState(NewState);
}

EVWeaponState::Type USeaCraftVehicleWeaponComponent::GetCurrentState() const
{
	return CurrentState;
}
