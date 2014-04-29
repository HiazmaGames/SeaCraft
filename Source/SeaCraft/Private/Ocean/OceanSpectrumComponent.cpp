// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "SeaCraft.h"
#include "UniformBuffer.h"
#include "ShaderParameters.h"

UOceanSpectrumComponent::UOceanSpectrumComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bAutoActivate = true;
	PrimaryComponentTick.bCanEverTick = true;
	//PrimaryComponentTick.TickGroup = TG_DuringPhysics;
}

void UOceanSpectrumComponent::SendRenderTransform_Concurrent()
{
	//UpdateContent();

	Super::SendRenderTransform_Concurrent();
}

void UOceanSpectrumComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// @TODO Update spectrum, etc.
	UpdateContent();
	UpdateDeferredSpectrums();
}

static TArray<UOceanSpectrumComponent*> OceanSpectrumsToUpdate;

void UOceanSpectrumComponent::UpdateContent()
{
	// Add check that ocean actor is visible to player
	if (World && World->Scene /**&& IsVisible()*/)
	{
		// Defer until after updates finish
		OceanSpectrumsToUpdate.AddUnique(this);
	}
}

void UOceanSpectrumComponent::UpdateDeferredSpectrums(/*FSceneInterface* Scene*/)
{
	for (int32 SpectrumIndex = 0; SpectrumIndex < OceanSpectrumsToUpdate.Num(); SpectrumIndex++)
	{
		UpdateOceanSpectrumContents(OceanSpectrumsToUpdate[SpectrumIndex]);
	}

	OceanSpectrumsToUpdate.Reset();
}

#if WITH_EDITOR
void UOceanSpectrumComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// AActor::PostEditChange will ForceUpdateComponents()
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateContent();
}
#endif // WITH_EDITOR

void UpdateOceanSpectrumContent_RenderThread(FTextureRenderTargetResource* TextureRenderTarget, FIntRect ViewRect, const FResolveParams& ResolveParams)
{
	// UE_LOG(LogOcean, Warning, TEXT("Update spectrum here"));

	RHISetRenderTarget(TextureRenderTarget->GetRenderTargetTexture(), NULL);
	RHIClear(true, FLinearColor::Yellow, false, 1.0f, false, 0, ViewRect);

	RHICopyToResolveTarget(TextureRenderTarget->GetRenderTargetTexture(), TextureRenderTarget->TextureRHI, false, ResolveParams);
}

void UOceanSpectrumComponent::UpdateOceanSpectrumContents(class UOceanSpectrumComponent* OceanSpectrumComponent)
{
	check(OceanSpectrumComponent);

	if (NormalsTarget && NormalsTarget->Resource)
	{
		FTextureRenderTargetResource* TextureRenderTarget = NormalsTarget->GameThread_GetRenderTargetResource();

		FIntPoint CaptureSize(TextureRenderTarget->GetSizeX(), TextureRenderTarget->GetSizeY());
		FIntRect ViewRect = FIntRect(0, 0, CaptureSize.X, CaptureSize.Y);

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			SpectrumRenderCommand,
			FTextureRenderTargetResource*, TextureRenderTarget, TextureRenderTarget,
			FIntRect, ViewRect, ViewRect,
			{
			UpdateOceanSpectrumContent_RenderThread(TextureRenderTarget, ViewRect, FResolveParams());
			});
	}
}

