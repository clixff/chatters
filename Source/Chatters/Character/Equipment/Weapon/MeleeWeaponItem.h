// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponItem.h"
#include "Sound/SoundBase.h"
#include "MeleeWeaponItem.generated.h"

UENUM()
enum class EMeleePhase : uint8
{
	IDLE,
	Hit
};

/**
 * 
 */
UCLASS()
class CHATTERS_API UMeleeWeaponItem : public UWeaponItem
{
	GENERATED_BODY()
public:
	UMeleeWeaponItem();
	~UMeleeWeaponItem();

	UPROPERTY(EditDefaultsOnly)
		FTransform CollisionTransform;

	UPROPERTY(EditDefaultsOnly)
		float HitTimeout = 2.5f;

	UPROPERTY(EditDefaultsOnly)
		float MaxDistance = 150.0f;

	UPROPERTY(EditDefaultsOnly)
		USoundBase* HitSound = nullptr;

	UPROPERTY(EditDefaultsOnly)
		USoundBase* DamageSound = nullptr;

	UPROPERTY(EditDefaultsOnly)
		FFloatRange AnimationTimeToDamage = FFloatRange(0.0f, 1.0f);
};
