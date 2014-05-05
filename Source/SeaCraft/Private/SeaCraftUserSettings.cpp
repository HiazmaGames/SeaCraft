// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraft.h"

static const auto StaticCVar = IConsoleManager::Get().RegisterConsoleVariable
	(
	TEXT("seacraft.graphics.quality"),
	-1,
	TEXT("Sets the graphics quality, need to execute ApplyUserSettings to apply\n")
	TEXT("-1:depends on settings (default,high)\n")
	TEXT("0:force low quality \n")
	TEXT("1:force mid quality \n")
	TEXT("2:force high quality"),
	ECVF_Default
	);

USeaCraftUserSettings::USeaCraftUserSettings(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	SetToDefaults();
}

void USeaCraftUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	bInvertedYAxis = false;
	AimSensitivity = 1.0f;
	Gamma = 2.2f;
	GraphicsQuality = 1;
}

void USeaCraftUserSettings::ApplySettings()
{
	Super::ApplySettings();

	// Graphics Quality
	{
		auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("seacraft.graphics.quality"));
		const int CVarValue = CVar->GetValueOnGameThread();

		int LocalGraphicsQuality = 0;

		if (CVarValue == -1)
		{
			LocalGraphicsQuality = GraphicsQuality;
		}
		else
		{
			LocalGraphicsQuality = FMath::Clamp(CVarValue, 0, 2);
		}

		UE_LOG(LogConsoleResponse, Display, TEXT("  GraphicsQuality %d"), LocalGraphicsQuality);

		if (LocalGraphicsQuality == 0)
		{
			SetLowQuality();
		}
		else if (LocalGraphicsQuality == 1)
		{
			SetMidQuality();
		}
		else if (LocalGraphicsQuality == 2)
		{
			SetHighQuality();
		}
	}

	if (!GEngine)
	{
		return;
	}


	TArray<APlayerController*> PlayerList;
	GEngine->GetAllLocalPlayerControllers(PlayerList);

	for (auto It = PlayerList.CreateIterator(); It; ++It)
	{
		APlayerController* PC = *It;
		if (!PC || !PC->PlayerInput)
		{
			continue;
		}

		//set the aim sensitivity
		for (int32 Idx = 0; Idx < PC->PlayerInput->AxisMappings.Num(); Idx++)
		{
			FInputAxisKeyMapping &AxisMapping = PC->PlayerInput->AxisMappings[Idx];
			if (AxisMapping.AxisName == "Lookup" || AxisMapping.AxisName == "Turn")
			{
				AxisMapping.Scale = (AxisMapping.Scale < 0.0f) ? -GetAimSensitivity() : +GetAimSensitivity();
			}
		}
		PC->PlayerInput->ForceRebuildingKeyMaps();

		//invert it, and if does not equal our bool, invert it again
		if (PC->PlayerInput->GetInvertAxis("Lookup") != GetInvertedYAxis())
		{
			PC->PlayerInput->InvertAxis("Lookup");
		}
	}

	GEngine->DisplayGamma = GetGamma();
}

bool USeaCraftUserSettings::IsAimSensitivityDirty() const
{
	bool bIsDirty = false;

	// Fixme: GameUserSettings is not setup to work with multiple worlds.
	// For now, user settings are global to all world instances.
	if (GEngine)
	{
		TArray<APlayerController*> PlayerList;
		GEngine->GetAllLocalPlayerControllers(PlayerList);

		for (auto It = PlayerList.CreateIterator(); It; ++It)
		{
			APlayerController* PC = *It;
			if (!PC || !PC->PlayerInput)
			{
				continue;
			}

			// check if the aim sensitivity is off anywhere
			for (int32 Idx = 0; Idx < PC->PlayerInput->AxisMappings.Num(); Idx++)
			{
				FInputAxisKeyMapping &AxisMapping = PC->PlayerInput->AxisMappings[Idx];
				if (AxisMapping.AxisName == "Lookup" || AxisMapping.AxisName == "Turn")
				{
					if (FMath::Abs(AxisMapping.Scale) != GetAimSensitivity())
					{
						bIsDirty = true;
						break;
					}
				}
			}
		}
	}
	return bIsDirty;
}

bool USeaCraftUserSettings::IsInvertedYAxisDirty() const
{
	bool bIsDirty = false;
	if (GEngine)
	{
		TArray<APlayerController*> PlayerList;
		GEngine->GetAllLocalPlayerControllers(PlayerList);

		for (auto It = PlayerList.CreateIterator(); It; ++It)
		{
			APlayerController* PC = *It;
			if (!PC || !PC->PlayerInput)
			{
				continue;
			}
		
			bIsDirty |= PC->PlayerInput->GetInvertAxis("Lookup") != GetInvertedYAxis();
		}
	}
	return bIsDirty;
}

bool USeaCraftUserSettings::IsDirty() const
{
	return Super::IsDirty() || IsAimSensitivityDirty() || IsInvertedYAxisDirty();
}

void USeaCraftUserSettings::ResetToCurrentSettings()
{
	Super::ResetToCurrentSettings();
	if (GEngine)
	{
		TArray<APlayerController*> PlayerList;
		GEngine->GetAllLocalPlayerControllers(PlayerList);

		for (auto It = PlayerList.CreateIterator(); It; ++It)
		{
			APlayerController* PC = *It;
			if (!PC || !PC->PlayerInput)
			{
				continue;
			}

			SetInvertedYAxis(PC->PlayerInput->GetInvertAxis("Lookup"));

			// Find a value for the aim sensitivity
			for (int32 Idx = 0; Idx < PC->PlayerInput->AxisMappings.Num(); Idx++)
			{
				FInputAxisKeyMapping &AxisMapping = PC->PlayerInput->AxisMappings[Idx];
				if (AxisMapping.AxisName == "Lookup" || AxisMapping.AxisName == "Turn")
				{
					SetAimSensitivity(FMath::Abs(AxisMapping.Scale));
					break;
				}
			}

			break;
		}
	}
}

int32 SeaCraftGetBoundFullScreenModeCVar()
{
	static const auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.FullScreenMode")); 

	if (FPlatformProperties::SupportsWindowedMode())
	{
		int32 Value = CVar->GetValueOnGameThread();

		if(Value >= 0 && Value <= 2)
		{
			return Value;
		}
	}

	// every other value behaves like 0
	return 0;
}

// depending on bFullscreen and the console variable r.FullScreenMode
EWindowMode::Type SeaCraftGetWindowModeType(bool bFullscreen)
{
	int32 FullScreenMode = SeaCraftGetBoundFullScreenModeCVar();

	if (FPlatformProperties::SupportsWindowedMode())
	{
		if(!bFullscreen)
		{
			return EWindowMode::Windowed;
		}

		if(FullScreenMode == 1 || FullScreenMode == 2)
		{
			return EWindowMode::WindowedFullscreen;
		}
	}

	return EWindowMode::Fullscreen;
}

EWindowMode::Type USeaCraftUserSettings::GetCurrentFullscreenMode() const
{
	EWindowMode::Type CurrentFullscreenMode = EWindowMode::Windowed;
	if ( GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportFrame )
	{
		bool bIsCurrentlyFullscreen = GEngine->GameViewport->IsFullScreenViewport();
		CurrentFullscreenMode = SeaCraftGetWindowModeType(bIsCurrentlyFullscreen);
	}
	return CurrentFullscreenMode;
}

void USeaCraftUserSettings::SetLowQuality()
{
	// Low-level options that may improve performance on low-end hardware.
	IConsoleVariable* CVarClearMethod = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ClearSceneMethod"));
	CVarClearMethod->Set( 0 );

	// Resolution quality
	IConsoleVariable* CScreenPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
	CScreenPercentage->Set( -1 );

	// Anti-aliasing quality
	IConsoleVariable* CVarAAQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessAAQuality"));
	CVarAAQuality->Set( 0 );

	// View Distance quality
	IConsoleVariable* CVarSkeletalLOD = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SkeletalMeshLODBias")); 
	static IConsoleVariable* CVarViewDistanceScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale"));
	CVarViewDistanceScale->Set( 0.6f );
	CVarSkeletalLOD->Set( 1 );

	// Shadow quality
	IConsoleVariable* CVarLightFunction = IConsoleManager::Get().FindConsoleVariable(TEXT("r.LightFunctionQuality")); 
	IConsoleVariable* CVarQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ShadowQuality")); 
	IConsoleVariable* CVarCascades = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.CSM.MaxCascades")); 
	IConsoleVariable* CVarRes = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.MaxResolution")); 
	IConsoleVariable* CVarCull = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.RadiusThreshold")); 
	IConsoleVariable* CDistScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.DistanceScale"));
	IConsoleVariable* CTransScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.CSM.TransitionScale")); 
	CVarLightFunction->Set(1);
	CVarQuality->Set(2);
	CVarCascades->Set(1);
	CVarRes->Set(1024);
	CVarCull->Set(0.05f);
	CDistScale->Set(0.7f);
	CTransScale->Set(0.25f);

	// Post Process quality
	IConsoleVariable* CVarMotionBlur = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality")); 
	IConsoleVariable* CVarSSAOLevels = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AmbientOcclusionLevels")); 
	IConsoleVariable* CVarSSAOScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AmbientOcclusionRadiusScale")); 
	IConsoleVariable* CVarDOF = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DepthOfFieldQuality")); 
	// smaller pool size is better to save GPU memory, higher number allows better reuse, less stalls on RT reallocations
	IConsoleVariable* CVarPoolMin = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RenderTargetPoolMin"));
	IConsoleVariable* CVarLensFlares = IConsoleManager::Get().FindConsoleVariable(TEXT("r.LensFlareQuality"));
	IConsoleVariable* CVarFringe = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SceneColorFringeQuality"));
	IConsoleVariable* CVarEyeAdaptation = IConsoleManager::Get().FindConsoleVariable(TEXT("r.EyeAdaptationQuality"));
	IConsoleVariable* CVarBloom = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BloomQuality"));
	CVarMotionBlur->Set(0);
	CVarSSAOLevels->Set(0);
	CVarSSAOScale->Set(1.7f);
	CVarDOF->Set(0);
	CVarPoolMin->Set(300);
	CVarLensFlares->Set(0);
	CVarFringe->Set(0);
	CVarEyeAdaptation->Set(0);
	CVarBloom->Set(4);

	// Texture quality
	IConsoleVariable* CVarMipBias = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.MipBias")); 
	IConsoleVariable* CVarAniso = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MaxAnisotropy")); 
	// 3.0 would make a seam appear at the player head
	CVarMipBias->Set(2.5f);
	CVarAniso->Set(0);

	// Lighting quality, maybe we should change to object/effect details
	IConsoleVariable* CVarVolumeDim = IConsoleManager::Get().FindConsoleVariable(TEXT("r.TranslucencyLightingVolumeDim")); 
	IConsoleVariable* CVarRefraction = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RefractionQuality")); 
	IConsoleVariable* CVarSSRQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SSR.Quality")); 
	IConsoleVariable* CVarColor = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SceneColorFormat")); 
	CVarVolumeDim->Set(24);
	CVarRefraction->Set(0);
	CVarSSRQuality->Set(0);
	CVarColor->Set(3);	// 32bit RGB float, reduced blending quality

	IConsoleManager::Get().CallAllConsoleVariableSinks();
}

void USeaCraftUserSettings::SetMidQuality()
{
	// Low-level options that may improve performance on low-end hardware.
	IConsoleVariable* CVarClearMethod = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ClearSceneMethod"));
	CVarClearMethod->Set(0);

	// Resolution quality
	IConsoleVariable* CScreenPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
	CScreenPercentage->Set(-1);

	// Anti-aliasing quality
	IConsoleVariable* CVarAAQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessAAQuality"));
	CVarAAQuality->Set(3);

	// View Distance quality
	IConsoleVariable* CVarSkeletalLOD = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SkeletalMeshLODBias"));
	static IConsoleVariable* CVarViewDistanceScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale"));
	CVarViewDistanceScale->Set(0.6f);
	CVarSkeletalLOD->Set(0);

	// Shadow quality
	IConsoleVariable* CVarLightFunction = IConsoleManager::Get().FindConsoleVariable(TEXT("r.LightFunctionQuality"));
	IConsoleVariable* CVarQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ShadowQuality"));
	IConsoleVariable* CVarCascades = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.CSM.MaxCascades"));
	IConsoleVariable* CVarRes = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.MaxResolution"));
	IConsoleVariable* CVarCull = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.RadiusThreshold"));
	IConsoleVariable* CDistScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.DistanceScale"));
	IConsoleVariable* CTransScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.CSM.TransitionScale"));
	CVarLightFunction->Set(1);
	CVarQuality->Set(3);
	CVarCascades->Set(2);
	CVarRes->Set(1024);
	CVarCull->Set(0.05f);
	CDistScale->Set(0.7f);
	CTransScale->Set(0.25f);

	// Post Process quality
	IConsoleVariable* CVarMotionBlur = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"));
	IConsoleVariable* CVarSSAOLevels = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AmbientOcclusionLevels"));
	IConsoleVariable* CVarSSAOScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AmbientOcclusionRadiusScale"));
	IConsoleVariable* CVarDOF = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DepthOfFieldQuality"));
	// smaller pool size is better to save GPU memory, higher number allows better reuse, less stalls on RT reallocations
	IConsoleVariable* CVarPoolMin = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RenderTargetPoolMin"));
	IConsoleVariable* CVarLensFlares = IConsoleManager::Get().FindConsoleVariable(TEXT("r.LensFlareQuality"));
	IConsoleVariable* CVarFringe = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SceneColorFringeQuality"));
	IConsoleVariable* CVarEyeAdaptation = IConsoleManager::Get().FindConsoleVariable(TEXT("r.EyeAdaptationQuality"));
	IConsoleVariable* CVarBloom = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BloomQuality"));
	CVarMotionBlur->Set(1);
	CVarSSAOLevels->Set(1);
	CVarSSAOScale->Set(1.7f);
	CVarDOF->Set(0);
	CVarPoolMin->Set(300);
	CVarLensFlares->Set(1);
	CVarFringe->Set(0);
	CVarEyeAdaptation->Set(0);
	CVarBloom->Set(4);

	// Texture quality
	IConsoleVariable* CVarMipBias = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.MipBias"));
	IConsoleVariable* CVarAniso = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MaxAnisotropy"));
	// 3.0 would make a seam appear at the player head
	CVarMipBias->Set(2.5f);
	CVarAniso->Set(0);

	// Lighting quality, maybe we should change to object/effect details
	IConsoleVariable* CVarVolumeDim = IConsoleManager::Get().FindConsoleVariable(TEXT("r.TranslucencyLightingVolumeDim"));
	IConsoleVariable* CVarRefraction = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RefractionQuality"));
	IConsoleVariable* CVarSSRQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SSR.Quality"));
	IConsoleVariable* CVarColor = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SceneColorFormat"));
	CVarVolumeDim->Set(24);
	CVarRefraction->Set(0);
	CVarSSRQuality->Set(1);
	CVarColor->Set(3);	// 32bit RGB float, reduced blending quality

	IConsoleManager::Get().CallAllConsoleVariableSinks();
}

void USeaCraftUserSettings::SetHighQuality()
{
	IConsoleVariable* CVarClearMethod = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ClearSceneMethod"));
	CVarClearMethod->Set( 1 );

	// Resolution quality
	IConsoleVariable* CScreenPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
	CScreenPercentage->Set( -1 );

	// Anti-aliasing quality
	IConsoleVariable* CVarAAQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessAAQuality"));
	CVarAAQuality->Set( 4 );

	// View Distance quality
	IConsoleVariable* CVarSkeletalLOD = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SkeletalMeshLODBias")); 
	static IConsoleVariable* CVarViewDistanceScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale"));
	CVarViewDistanceScale->Set( 1.0f );
	CVarSkeletalLOD->Set( 0 );

	// Shadow quality
	IConsoleVariable* CVarLightFunction = IConsoleManager::Get().FindConsoleVariable(TEXT("r.LightFunctionQuality")); 
	IConsoleVariable* CVarQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ShadowQuality")); 
	IConsoleVariable* CVarCascades = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.CSM.MaxCascades")); 
	IConsoleVariable* CVarRes = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.MaxResolution")); 
	IConsoleVariable* CVarCull = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.RadiusThreshold")); 
	IConsoleVariable* CDistScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.DistanceScale"));
	IConsoleVariable* CTransScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.CSM.TransitionScale")); 
	CVarLightFunction->Set(2);
	CVarQuality->Set(5);
	CVarCascades->Set(4);
	CVarRes->Set(1024);	
	CVarCull->Set(0.03f);
	CDistScale->Set(1.0f);
	CTransScale->Set(1.0f);

	// Post Process quality
	IConsoleVariable* CVarMotionBlur = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality")); 
	IConsoleVariable* CVarSSAOLevels = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AmbientOcclusionLevels")); 
	IConsoleVariable* CVarSSAOScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AmbientOcclusionRadiusScale")); 
	IConsoleVariable* CVarDOF = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DepthOfFieldQuality")); 
	// smaller pool size is better to save GPU memory, higher number allows better reuse, less stalls on RT reallocations
	IConsoleVariable* CVarPoolMin = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RenderTargetPoolMin"));
	IConsoleVariable* CVarLensFlares = IConsoleManager::Get().FindConsoleVariable(TEXT("r.LensFlareQuality"));
	IConsoleVariable* CVarFringe = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SceneColorFringeQuality"));
	IConsoleVariable* CVarEyeAdaptation = IConsoleManager::Get().FindConsoleVariable(TEXT("r.EyeAdaptationQuality"));
	IConsoleVariable* CVarBloom = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BloomQuality"));
	CVarMotionBlur->Set(4);
	CVarSSAOLevels->Set(3);
	CVarSSAOScale->Set(1.0f);
	CVarDOF->Set(2);
	CVarPoolMin->Set(400);
	CVarLensFlares->Set(2);
	CVarFringe->Set(1);
	CVarEyeAdaptation->Set(2);
	CVarBloom->Set(5);

	// Texture quality
	IConsoleVariable* CVarMipBias = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.MipBias")); 
	IConsoleVariable* CVarAniso = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MaxAnisotropy")); 
	CVarMipBias->Set(0.0f);
	CVarAniso->Set(4);

	// Lighting quality, maybe we should change to object/effect details
	IConsoleVariable* CVarVolumeDim = IConsoleManager::Get().FindConsoleVariable(TEXT("r.TranslucencyLightingVolumeDim")); 
	IConsoleVariable* CVarRefraction = IConsoleManager::Get().FindConsoleVariable(TEXT("r.RefractionQuality")); 
	IConsoleVariable* CVarSSRQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SSR.Quality")); 
	IConsoleVariable* CVarColor = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SceneColorFormat")); 
	CVarVolumeDim->Set(64);
	CVarRefraction->Set(2);
	CVarSSRQuality->Set(1);
	CVarColor->Set(4);	// fp16 RGB color, A unused, high quality blending}

	IConsoleManager::Get().CallAllConsoleVariableSinks();
}
