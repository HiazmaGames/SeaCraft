// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "SeaCraft.h"
#include "UniformBuffer.h"

#define HALF_SQRT_2	0.7071068f
#define GRAV_ACCEL	981.0f	// The acceleration of gravity, cm/s^2

#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16

/** Generating gaussian random number with mean 0 and standard deviation 1 */
float Gauss()
{
	float u1 = FMath::SRand();
	float u2 = FMath::SRand();

	if (u1 < 1e-6f)
	{
		u1 = 1e-6f;
	}

	return sqrtf(-2 * logf(u1)) * cosf(2 * PI * u2);
}

/**
 * Phillips Spectrum
 * K: normalized wave vector, W: wind direction, v: wind velocity, a: amplitude constant
 */
float Phillips(FVector2D K, FVector2D W, float v, float a, float dir_depend)
{
	// Largest possible wave from constant wind of velocity v
	float l = v * v / GRAV_ACCEL;

	// Damp out waves with very small length w << l
	float w = l / 1000;

	float Ksqr = K.X * K.X + K.Y * K.Y;
	float Kcos = K.X * W.X + K.Y * W.Y;
	float phillips = a * expf(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);

	// Filter out waves moving opposite to wind
	if (Kcos < 0)
	{
		phillips *= dir_depend;
	}

	// Damp out waves with very small length w << l
	return phillips * expf(-Ksqr * w * w);
}


//////////////////////////////////////////////////////////////////////////
// Spectrum component

UOceanSpectrumComponent::UOceanSpectrumComponent(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	bAutoActivate = true;
	PrimaryComponentTick.bCanEverTick = true;
	//PrimaryComponentTick.TickGroup = TG_DuringPhysics;

	if (NormalsTarget)
	{
		NormalsTarget->bForceLinearGamma = true;
	}

	if (HeightTarget)
	{
		HeightTarget->bForceLinearGamma = true;
	}

	// Cache shader immutable parameters
	g_ActualDim = OceanConfig.DispMapDimension;
	g_InWidth = g_ActualDim + 4;
	g_OutWidth = g_ActualDim;
	g_OutHeight = g_ActualDim;

	// Black color to fill the array
	FFloat16Color BlackColor;
	BlackColor.R = 0;
	BlackColor.G = 0;
	BlackColor.B = 0;
	BlackColor.A = 0;

	// Height map H(0)
	//int32 height_map_size = FMath::Square(OceanConfig.DispMapDimension);
	int32 height_map_size = (OceanConfig.DispMapDimension + 4) * (OceanConfig.DispMapDimension + 1);

	h0_data.Empty();
	h0_data.Init(BlackColor, height_map_size);

	omega_data.Empty();
	omega_data.Init(0.0f, height_map_size);

	InitHeightMap(OceanConfig, h0_data, omega_data);

	// This value should be (hmap_dim / 2 + 1) * hmap_dim, but we use full sized buffer here for simplicity
	int32 input_half_size = FMath::Square(OceanConfig.DispMapDimension);

	// H(t), Dx(t) and Dy(t)
	Ht_data.Empty();
	Ht_data.Init(BlackColor, input_half_size);

	Ht_Dx.Empty();
	Ht_Dx.Init(BlackColor, input_half_size);

	Ht_Dy.Empty();
	Ht_Dy.Init(BlackColor, input_half_size);

	// Update maps for the first time
	UpdateDisplacementMap(0);
}

void UOceanSpectrumComponent::InitHeightMap(FOceanData& Params, TResourceArray<FFloat16Color>& out_h0, TResourceArray<float>& out_omega)
{
	int32 i, j;
	FVector2D K, Kn;

	FVector2D wind_dir = Params.WindDirection;
	wind_dir.Normalize();

	float a = Params.WaveAmplitude * 1e-7f;	// It is too small. We must scale it for editing.
	float v = Params.WindSpeed;
	float dir_depend = Params.WindDependency;

	int height_map_dim = Params.DispMapDimension;
	float patch_length = Params.PatchLength;

	for (i = 0; i <= height_map_dim; i++)
	{
		// K is wave-vector, range [-|DX/W, |DX/W], [-|DY/H, |DY/H]
		K.Y = (-height_map_dim / 2.0f + i) * (2 * PI / patch_length);

		for (j = 0; j <= height_map_dim; j++)
		{
			K.X = (-height_map_dim / 2.0f + j) * (2 * PI / patch_length);

			float phil = (K.X == 0 && K.Y == 0) ? 0 : sqrtf(Phillips(K, wind_dir, v, a, dir_depend));

			out_h0[i * (height_map_dim + 4) + j].R = FFloat16(phil * Gauss() * HALF_SQRT_2);
			out_h0[i * (height_map_dim + 4) + j].G = FFloat16(phil * Gauss() * HALF_SQRT_2);

			//UE_LOG(LogOcean, Warning, TEXT("Update spectrum here %d"), i * height_map_dim + j);

			// The angular frequency is following the dispersion relation:
			//            out_omega^2 = g*k
			// The equation of Gerstner wave:
			//            x = x0 - K/k * A * sin(dot(K, x0) - sqrt(g * k) * t), x is a 2D vector.
			//            z = A * cos(dot(K, x0) - sqrt(g * k) * t)
			// Gerstner wave shows that a point on a simple sinusoid wave is doing a uniform circular
			// motion with the center (x0, y0, z0), radius A, and the circular plane is parallel to
			// vector K.
			out_omega[i * (height_map_dim + 4) + j] = sqrtf(GRAV_ACCEL * sqrtf(K.X * K.X + K.Y * K.Y));
		}
	}
}

void UOceanSpectrumComponent::CreateBufferAndUAV(FResourceArrayInterface* Data, uint32 byte_width, uint32 byte_stride,
	FStructuredBufferRHIRef& ppBuffer, FUnorderedAccessViewRHIRef& ppUAV, FShaderResourceViewRHIRef& ppSRV)
{
	ppBuffer = RHICreateStructuredBuffer(byte_stride, Data->GetResourceDataSize(), Data, (BUF_UnorderedAccess | BUF_ShaderResource));
	ppUAV = RHICreateUnorderedAccessView(ppBuffer, false, false);
	ppSRV = RHICreateShaderResourceView(ppBuffer);
}


//////////////////////////////////////////////////////////////////////////
// Displacement map update

void UOceanSpectrumComponent::SendRenderTransform_Concurrent()
{
	UpdateContent();

	Super::SendRenderTransform_Concurrent();
}

void UOceanSpectrumComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//UpdateDisplacementMap(GetWorld()->GetTimeSeconds());

	// Finaly update render targets content
	UpdateOceanSpectrumContents(this);
}

void UOceanSpectrumComponent::UpdateDisplacementMap(float WorldTime)
{
	// Update per frame data
	g_Time = WorldTime * OceanConfig.TimeScale;
	g_ChoppyScale = OceanConfig.ChoppyScale;

	// H(0) -> H(t), D(x,t), D(y,t)
	for (int i = 0; i < g_ActualDim; i++)
	{
		for (int j = 0; j < g_ActualDim; j++)
		{
			// Can be run parrallel in threads
			UpdateSpectrum(FIntVector(i, j, 0));
		}
	}
}

void UOceanSpectrumComponent::UpdateSpectrum(FIntVector DTid)
{
	int in_index = DTid.Y * g_InWidth + DTid.X;
	int in_mindex = (g_ActualDim - DTid.Y) * g_InWidth + (g_ActualDim - DTid.X);
	int out_index = DTid.Y * g_OutWidth + DTid.X;

	// H(0) -> H(t)
	FVector2D h0_k = FVector2D(h0_data[in_index].R, h0_data[in_index].G);
	FVector2D h0_mk = FVector2D(h0_data[in_mindex].R, h0_data[in_mindex].G);
	float sin_v, cos_v;
	sin_v = FMath::Sin(omega_data[in_index] * g_Time);
	cos_v = FMath::Cos(omega_data[in_index] * g_Time);

	FVector2D ht;
	ht.X = (h0_k.X + h0_mk.X) * cos_v - (h0_k.Y + h0_mk.Y) * sin_v;
	ht.Y = (h0_k.X - h0_mk.X) * sin_v + (h0_k.Y - h0_mk.Y) * cos_v;

	// H(t) -> Dx(t), Dy(t)
	float kx = DTid.X - g_ActualDim * 0.5f;
	float ky = DTid.X - g_ActualDim * 0.5f;
	float sqr_k = kx * kx + ky * ky;
	float rsqr_k = 0;
	if (sqr_k > 1e-12f)
	{
		rsqr_k = 1 / sqrt(sqr_k);
	}
	
	kx *= rsqr_k;
	ky *= rsqr_k;
	FVector2D dt_x = FVector2D(ht.Y * kx, -ht.X * kx);
	FVector2D dt_y = FVector2D(ht.Y * ky, -ht.X * ky);

	if ((DTid.X < g_OutWidth) && (DTid.Y < g_OutHeight))
	{
		Ht_data[out_index].R = FFloat16(ht.X);
		Ht_data[out_index].G = FFloat16(ht.Y);

		Ht_Dx[out_index].R = FFloat16(dt_x.X);
		Ht_Dx[out_index].G = FFloat16(dt_x.Y);

		Ht_Dy[out_index].R = FFloat16(dt_y.X);
		Ht_Dy[out_index].G = FFloat16(dt_y.Y);
	}
}


//////////////////////////////////////////////////////////////////////////
// Render targets update

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


//////////////////////////////////////////////////////////////////////////
// Spectrum RT rendering

void UpdateOceanSpectrumContent_RenderThread(FTextureRenderTargetResource* TextureRenderTarget, FIntRect ViewRect, const FResolveParams& ResolveParams)
{
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

		ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
			SpectrumRenderCommand,
			FTextureRenderTargetResource*, TextureRenderTarget, TextureRenderTarget,
			FIntRect, ViewRect, ViewRect,
			FResourceArrayInterface*, Data, &h0_data,
			{
				uint32 stride = 0;
				void* MipData = GDynamicRHI->RHILockTexture2D(TextureRenderTarget->GetRenderTargetTexture(), 0, RLM_WriteOnly, stride, false);
				if (MipData)
				{
					//FMemory::Memcpy(MipData, Data->GetResourceData(), Data->GetResourceDataSize());
					GDynamicRHI->RHIUnlockTexture2D(TextureRenderTarget->GetRenderTargetTexture(), 0, false);
				}
			});
	}
}
