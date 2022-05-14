// Fill out your copyright notice in the Description page of Project Settings.


#include "BloodDecal.h"
#include "../Character/Bot.h"

// Sets default values
ABloodDecal::ABloodDecal()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	this->SetRootComponent(this->DefaultSceneRoot);

	this->Decal = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	this->Decal->SetupAttachment(this->DefaultSceneRoot);
}

// Called when the game starts or when spawned
void ABloodDecal::BeginPlay()
{
	Super::BeginPlay();
	
	if (this->DecalMaterialBase)
	{
		this->DecalMaterial = UMaterialInstanceDynamic::Create(this->DecalMaterialBase, this);

		if (!this->DecalMaterial)
		{
			return;
		}

		if (!this->Decal)
		{
			return;
		}

		if (OpacityMasksList.Num())
		{
			auto* OpacityMask = OpacityMasksList[FMath::RandRange(0, OpacityMasksList.Num() - 1)];
			if (OpacityMask)
			{
				DecalMaterial->SetTextureParameterValue(TEXT("Mask"), OpacityMask);
			}
		}

		this->Decal->SetDecalMaterial(this->DecalMaterial);
	}
}

// Called every frame
void ABloodDecal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LifeTime += DeltaTime;

	if (!this->DecalMaterial)
	{
		return;
	}

	if (!this->OpacityCurve)
	{
		return;
	}

	auto* OpacityCurveRef = bFloorDecal ? OpacityCurve : WallOpacityCurve;
	auto* ColorMultiplierRef = bFloorDecal ? ColorMultiplierCurve : WallColorMultiplierCurve;
	
	float ColorMultiplier = ColorMultiplierRef->GetFloatValue(LifeTime);
	float OpacityValue = OpacityCurveRef->GetFloatValue(LifeTime);

	this->DecalMaterial->SetScalarParameterValue(TEXT("Multiplier"), ColorMultiplier);
	this->DecalMaterial->SetScalarParameterValue(TEXT("Opacity"), OpacityValue);

	if (OpacityValue <= 0.0f)
	{
		this->DestroyDecal();
	}

}

void ABloodDecal::DestroyDecal()
{
	if (this->BotOwner)
	{
		ABot* Bot = Cast<ABot>(this->BotOwner);

		if (Bot)
		{
			if (bFloorDecal)
			{
				Bot->RemoveBloodDecal();
			}
			else
			{
				Bot->RemoveWallBloodDecal(this);
			}
		}
	}
}

