// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

#include "SeaCraft.generated.inl"

class FSeaCraftGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
		
	}

	virtual void ShutdownModule() override
	{
		
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FSeaCraftGameModule, SeaCraft, "SeaCraft" );

DEFINE_LOG_CATEGORY(LogShipPhysics);
DEFINE_LOG_CATEGORY(LogOcean);
DEFINE_LOG_CATEGORY(LogWeapon);
