// Copyright 2011-2014 UFNA, LLC. All Rights Reserved.

#include "SeaCraftUserSettings.generated.h"

UCLASS()
class USeaCraftUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()

	/** Applies all current user settings to the game and saves to permanent storage (e.g. file). */
	virtual void ApplySettings() OVERRIDE;

	/** Checks if any user settings is different from current */
	virtual bool IsDirty() const OVERRIDE;

	/** Is the y axis inverted? */
	bool GetInvertedYAxis() const
	{
		return bInvertedYAxis;
	}

	/** Setter for inverted y axis */
	void SetInvertedYAxis(bool bInvert)
	{
		bInvertedYAxis = bInvert;
	}

	/** Getter for the aim sensitivity */
	float GetAimSensitivity() const
	{
		return AimSensitivity;
	}

	void SetAimSensitivity(float InSensitivity)
	{
		AimSensitivity = InSensitivity;
	}

	/** Getter for the gamma correction */
	float GetGamma() const
	{
		return Gamma;
	}

	void SetGamma(float InGamma)
	{
		Gamma = InGamma;
	}

	int32 GetGraphicsQuality() const
	{
		return GraphicsQuality;
	}

	void SetGraphicsQuality(int32 InGraphicsQuality)
	{
		GraphicsQuality = InGraphicsQuality;
	}

	/** Checks if the Mouse Sensitivity user setting is different from current */
	bool IsAimSensitivityDirty() const;

	/** Checks if the Inverted Mouse user setting is different from current */
	bool IsInvertedYAxisDirty() const;

	/** Gets current fullscreen mode */
	EWindowMode::Type GetCurrentFullscreenMode() const;

	/** This function resets all settings to the current system settings */
	virtual void ResetToCurrentSettings() OVERRIDE;

	/** Interface UGameUserSettings */
	virtual void SetToDefaults() OVERRIDE;

	/** Changes to low quality graphics */
	void SetLowQuality();

	/** Changes to mid quality graphics */
	void SetMidQuality();

	/** Changes to high quality graphics */
	void SetHighQuality();

private:
	/** Is the y axis inverted or not? */
	UPROPERTY(config)
	bool bInvertedYAxis;

	/** Holds the mouse sensitivity */
	UPROPERTY(config)
	float AimSensitivity;

	/** Holds the gamma correction setting */
	UPROPERTY(config)
	float Gamma;

	/**
	 * Graphics Quality
	 *	0 = Low
	 *	1 = High
	 */
	UPROPERTY(config)
	int32 GraphicsQuality;

};
