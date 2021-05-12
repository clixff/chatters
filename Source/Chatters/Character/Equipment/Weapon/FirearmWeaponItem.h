// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponItem.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "FirearmWeaponItem.generated.h"


/**
 * 
 */
UCLASS()
class CHATTERS_API UFirearmWeaponItem : public UWeaponItem
{
	GENERATED_BODY()
public:
	UFirearmWeaponItem();
	~UFirearmWeaponItem();

	UPROPERTY(EditDefaultsOnly)
		float MaxDistance = 1000.0f;

	UPROPERTY(EditDefaultsOnly)
		int32 MaxNumberOfBullets = 10;

	/** In seconds */
	UPROPERTY(EditDefaultsOnly)
		float ReloadingTime = 4.0f;

	/** In seconds */
	UPROPERTY(EditDefaultsOnly)
		float ShootTime = 1.0f;

	UPROPERTY(EditDefaultsOnly)
		USoundBase* ShootSound;

	UPROPERTY(EditDefaultsOnly)
		float ImpulseForce = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category="Transform")
		FVector SocketRelativeLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Particle")
		UParticleSystem* ShotParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Particle")
		FVector ParticleScale = FVector(1.0f);

};
