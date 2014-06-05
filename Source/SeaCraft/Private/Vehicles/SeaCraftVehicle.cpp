// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

FName ASeaCraftVehicle::VehicleMeshComponentName(TEXT("SeaCraftVehicleMesh"));

ASeaCraftVehicle::ASeaCraftVehicle(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	VehicleMesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, VehicleMeshComponentName);
	static FName CollisionProfileName(TEXT("Vehicle"));
	VehicleMesh->SetCollisionProfileName(CollisionProfileName);
	VehicleMesh->BodyInstance.bSimulatePhysics = true;
	VehicleMesh->BodyInstance.bNotifyRigidBodyCollision = true;
	VehicleMesh->BodyInstance.bUseCCD = true;
	VehicleMesh->bBlendPhysics = true;
	//VehicleMesh->bUseSingleBodyPhysics = true;
	VehicleMesh->bGenerateOverlapEvents = true;
	RootComponent = VehicleMesh;

	bWantsToFire = false;

	Health = 100;
}

void ASeaCraftVehicle::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Role == ROLE_Authority)
	{
		Health = GetMaxHealth();

		// Activate first weapon group
		TArray<USeaCraftVehicleWeaponComponent*> Components;
		this->GetComponents<USeaCraftVehicleWeaponComponent>(Components);

		for (int i = 0; i < Components.Num(); i++)
		{
			WeaponGroups.AddUnique(Components[i]->GetGroupID());
		}

		if (WeaponGroups.Num() > 0)
		{
			SetWeaponGroup(WeaponGroups[0]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Replication

void ASeaCraftVehicle::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	// Only replicate this property for a short duration after it changes so join in progress players don't get spammed with fx when joining late
	DOREPLIFETIME_ACTIVE_OVERRIDE(ASeaCraftVehicle, LastTakeHitInfo, GetWorld() && GetWorld()->GetTimeSeconds() < LastTakeHitTimeTimeout);
}

void ASeaCraftVehicle::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Only to local owner: weapon change requests are locally instigated, other clients don't need it
	// DOREPLIFETIME_CONDITION(ASeaCraftVehicle, Inventory, COND_OwnerOnly);

	DOREPLIFETIME_CONDITION(ASeaCraftVehicle, LastTakeHitInfo, COND_Custom);

	// Everyone
	//DOREPLIFETIME(ASeaCraftVehicle, CurrentWeapon);
	DOREPLIFETIME(ASeaCraftVehicle, Health);
}


//////////////////////////////////////////////////////////////////////////
// Damage & death

void ASeaCraftVehicle::FellOutOfWorld(const class UDamageType& dmgType)
{
	Die(Health, FDamageEvent(dmgType.GetClass()), NULL, NULL);
}

void ASeaCraftVehicle::Suicide()
{
	KilledBy(this);
}

void ASeaCraftVehicle::KilledBy(APawn* EventInstigator)
{
	if (Role == ROLE_Authority && !bIsDying)
	{
		AController* Killer = NULL;
		if (EventInstigator != NULL)
		{
			Killer = EventInstigator->Controller;
			LastHitBy = NULL;
		}

		Die(Health, FDamageEvent(UDamageType::StaticClass()), Killer, NULL);
	}
}

float ASeaCraftVehicle::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	// @TODO Check GodMode (for admin)
	/*ASeaCraftPlayerController* MyPC = Cast<ASeaCraftPlayerController>(Controller);
	if (MyPC && MyPC->HasGodMode())
	{
		return 0.f;
	}*/

	if (Health <= 0.f)
	{
		return 0.f;
	}

	// @TODO Modify based on game rules.
	//ASeaCraftGameMode* const Game = GetWorld()->GetAuthGameMode<ASeaCraftGameMode>();
	//Damage = Game ? Game->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : 0.f;

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		Health -= ActualDamage;
		if (Health <= 0)
		{
			Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			PlayHit(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
		}

		MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);
	}

	return ActualDamage;
}


bool ASeaCraftVehicle::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const
{
	if (bIsDying										// already dying
		|| IsPendingKill()								// already destroyed
		|| Role != ROLE_Authority						// not authority
		|| GetWorld()->GetAuthGameMode() == NULL
		/*|| GetWorld()->GetAuthGameMode()->bLevelChange*/)	// level transition occurring
	{
		return false;
	}

	return true;
}


bool ASeaCraftVehicle::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	Health = FMath::Min(0.0f, Health);

	// If this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	// @TODO Notify game mode
	// AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	// GetWorld()->GetAuthGameMode<ASeaCraftGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	NetUpdateFrequency = GetDefault<ASeaCraftVehicle>()->NetUpdateFrequency;

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}


void ASeaCraftVehicle::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	bReplicateMovement = false;
	bTearOff = true;
	bIsDying = true;

	if (Role == ROLE_Authority)
	{
		ReplicateHit(KillingDamage, DamageEvent, PawnInstigator, DamageCauser, true);

		// play the force feedback effect on the client player controller
		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			// @TODO Play force feedback
			/*USeaCraftDamageType *DamageType = Cast<USeaCraftDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->KilledForceFeedback)
			{
				PC->ClientPlayForceFeedback(DamageType->KilledForceFeedback, false, "Damage");
			}*/
		}
	}

	// @TODO Play death sound
	// Cannot use IsLocallyControlled here, because even local client's controller may be NULL here
	/*if (GetNetMode() != NM_DedicatedServer && DeathSound && Mesh1P && Mesh1P->IsVisible())
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}*/

	// @TODO Should we detach player to make him spectate?
	//DetachFromControllerPendingDestroy();
}

void ASeaCraftVehicle::PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (Role == ROLE_Authority)
	{
		ReplicateHit(DamageTaken, DamageEvent, PawnInstigator, DamageCauser, false);

		// @TODO Play the force feedback effect on the client player controller
		/*APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			USeaCraftDamageType *DamageType = Cast<USeaCraftDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->HitForceFeedback)
			{
				PC->ClientPlayForceFeedback(DamageType->HitForceFeedback, false, "Damage");
			}
		}*/
	}

	// @TODO Notify HUDs about hit
	/*ASeaCraftPlayerController* MyPC = Cast<ASeaCraftPlayerController>(Controller);
	ASeaCraftHUD* MyHUD = MyPC ? Cast<ASeaCraftHUD>(MyPC->GetHUD()) : NULL;
	if (MyHUD)
	{
		MyHUD->NotifyHit(DamageTaken, DamageEvent, PawnInstigator);
	}

	if (PawnInstigator && PawnInstigator != this && PawnInstigator->IsLocallyControlled())
	{
		ASeaCraftPlayerController* InstigatorPC = Cast<ASeaCraftPlayerController>(PawnInstigator->Controller);
		ASeaCraftHUD* InstigatorHUD = InstigatorPC ? Cast<ASeaCraftHUD>(InstigatorPC->GetHUD()) : NULL;
		if (InstigatorHUD)
		{
			InstigatorHUD->NotifyEnemyHit();
		}
	}*/
}

void ASeaCraftVehicle::ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser, bool bKilled)
{
	const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	FDamageEvent const& LastDamageEvent = LastTakeHitInfo.GetDamageEvent();
	if ((PawnInstigator == LastTakeHitInfo.PawnInstigator.Get()) && (LastDamageEvent.DamageTypeClass == LastTakeHitInfo.DamageTypeClass) && (LastTakeHitTimeTimeout == TimeoutTime))
	{
		// same frame damage
		if (bKilled && LastTakeHitInfo.bKilled)
		{
			// Redundant death take hit, just ignore it
			return;
		}

		// otherwise, accumulate damage done this frame
		Damage += LastTakeHitInfo.ActualDamage;
	}

	LastTakeHitInfo.ActualDamage = Damage;
	LastTakeHitInfo.PawnInstigator = Cast<ASeaCraftVehicle>(PawnInstigator);
	LastTakeHitInfo.DamageCauser = DamageCauser;
	LastTakeHitInfo.SetDamageEvent(DamageEvent);
	LastTakeHitInfo.bKilled = bKilled;
	LastTakeHitInfo.EnsureReplication();

	LastTakeHitTimeTimeout = TimeoutTime;
}

void ASeaCraftVehicle::OnRep_LastTakeHitInfo()
{
	if (LastTakeHitInfo.bKilled)
	{
		OnDeath(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
	else
	{
		PlayHit(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
}

//Pawn::PlayDying sets this lifespan, but when that function is called on client, dead pawn's role is still SimulatedProxy despite bTearOff being true. 
void ASeaCraftVehicle::TornOff()
{
	SetLifeSpan(25.f);
}


//////////////////////////////////////////////////////////////////////////
// Reading data

float ASeaCraftVehicle::GetHealth() const
{
	return Health;
}

int32 ASeaCraftVehicle::GetMaxHealth() const
{
	return GetClass()->GetDefaultObject<ASeaCraftVehicle>()->Health;
}

bool ASeaCraftVehicle::IsAlive() const
{
	return Health > 0;
}


//////////////////////////////////////////////////////////////////////////
// Weapon control

void ASeaCraftVehicle::SetWeaponGroup(FName WeaponGroup)
{
	// Stop current weapons before changing group
	StopWeaponFire();

	CurrentWeaponGroup = WeaponGroup;
	CurrentWeapons.Empty();

	TArray<USeaCraftVehicleWeaponComponent*> Components;
	this->GetComponents<USeaCraftVehicleWeaponComponent>(Components);

	for (int i = 0; i < Components.Num(); i++)
	{
		if (Components[i]->GetGroupID() == CurrentWeaponGroup)
		{
			CurrentWeapons.Add(Components[i]);
		}
	}
}

int32 ASeaCraftVehicle::FindWeaponGroup(const FName& WeaponGroup)
{
	for (int i = 0; i < WeaponGroups.Num(); i++)
	{
		if (WeaponGroups[i] == CurrentWeaponGroup)
		{
			return i;
		}
	}

	return -1;
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

void ASeaCraftVehicle::StartWeaponFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		
		for (USeaCraftVehicleWeaponComponent* Weapon : CurrentWeapons)
		{
			Weapon->StartFire();
		}
	}
}

void ASeaCraftVehicle::StopWeaponFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		
		for (USeaCraftVehicleWeaponComponent* Weapon : CurrentWeapons)
		{
			Weapon->StopFire();
		}
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

	UE_LOG(LogWeapon, Warning, TEXT("WEAPON: Input action binded"));
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
	if (WeaponGroups.Num() == 0)
	{
		return;
	}

	int32 CurrentIdx = FindWeaponGroup(CurrentWeaponGroup);
	int32 NewIndex = CurrentIdx + 1;

	if (!WeaponGroups.IsValidIndex(NewIndex))
	{
		NewIndex = 0;
	}

	SetWeaponGroup(WeaponGroups[NewIndex]);
}

void ASeaCraftVehicle::OnPrevWeapon()
{
	if (WeaponGroups.Num() == 0)
	{
		return;
	}

	int32 CurrentIdx = FindWeaponGroup(CurrentWeaponGroup);
	int32 NewIndex = CurrentIdx - 1;

	if (!WeaponGroups.IsValidIndex(NewIndex))
	{
		NewIndex = WeaponGroups.Num() - 1;
	}

	SetWeaponGroup(WeaponGroups[NewIndex]);
}
