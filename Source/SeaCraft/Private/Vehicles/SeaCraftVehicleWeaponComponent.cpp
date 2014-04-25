// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

USeaCraftVehicleWeaponComponent::USeaCraftVehicleWeaponComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bWantsToFire = false;
	CurrentState = EVWeaponState::Idle;

	CurrentAmmo = 0;
	TimeBetweenShots = 0.2f;
	LastFireTime = 0.0f;
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
// Ammo

void USeaCraftVehicleWeaponComponent::GiveAmmo(int AddAmount)
{
	// @TODO Check max ammo capacity reached
	CurrentAmmo += AddAmount;
}

void USeaCraftVehicleWeaponComponent::UseAmmo()
{
	if (!HasInfiniteAmmo())
	{
		CurrentAmmo--;
	}
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

void USeaCraftVehicleWeaponComponent::HandleFiring()
{
	APawn* MyPawn = Cast<APawn>(GetOwner());

	if ((CurrentAmmo > 0 || HasInfiniteAmmo()) && CanFire())
	{
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			FireWeapon();

			UseAmmo();

			// @TODO Send vehicle event to trigger effects
		}
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (GetCurrentAmmo() == 0 && !bRefiring)
		{
			// TODO Notify out of ammo
		}
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		// Local client will notify server
		if (MyPawn->Role < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// Setup refire timer
		bRefiring = (CurrentState == EVWeaponState::Firing && TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			MyPawn->GetWorldTimerManager().SetTimer(this, &USeaCraftVehicleWeaponComponent::HandleFiring, TimeBetweenShots, false);
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
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

int32 USeaCraftVehicleWeaponComponent::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

bool USeaCraftVehicleWeaponComponent::HasInfiniteAmmo() const
{
	return false;
}

void USeaCraftVehicleWeaponComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(USeaCraftVehicleWeaponComponent, CurrentAmmo, COND_OwnerOnly);
}
