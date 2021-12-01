// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponItem.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "../../../Combat/FirearmProjectile.h"
#include "NiagaraSystem.h"
#include "../../../Misc/Animation/BowstringAnimation.h"
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
		bool bCanMoveWhenShooting = true;

	UPROPERTY(EditDefaultsOnly, Category="Transform")
		FVector SocketRelativeLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Particle")
		UParticleSystem* ShotParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Particle")
		FVector ParticleScale = FVector(1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TSubclassOf<AFirearmProjectile> FirearmProjectileSubClass = AFirearmProjectile::StaticClass();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<FLinearColor> RandomProjectileColors;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<FLinearColor> TeamProjectileColors;

	FLinearColor GetRandomProjectileColor();

	UPROPERTY(EditDefaultsOnly, Category = "Reloading particle")
		UNiagaraSystem* ReloadingParticle = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Reloading particle")
		FTransform ReloadingParticleTransform;

	UPROPERTY(EditDefaultsOnly, Category = "Reloading particle")
		float ReloadingParticleStartSecond = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile Mesh")
		UStaticMesh* ProjectileStaticMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile Mesh")
		FTransform ProjectileMeshTransform;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile Mesh")
		float ProjectileMeshRespawnTime = 0.5f;

	/** For example, arrow */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile Mesh")
		bool bCanProjectileMeshBeAttachedToEnemy = false;

	UPROPERTY(EditDefaultsOnly, Category = "Bowstring")
		USkeletalMesh* BowstringMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Bowstring")
		TSubclassOf<UBowstringAnimation> BowstringAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Bowstring")
		float BowstringDetachTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Bowstring")
		float BowstringAttachTime = 1.5f;
};
