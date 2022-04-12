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

USTRUCT(BlueprintType)
struct FMeleeAnimation
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UAnimSequence* DamageAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float AnimationTimeSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FFloatRange AnimationTimeToDamage = FFloatRange(0.0f, 1.0f);
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
		TArray<FMeleeAnimation> HitAnimations;

	FMeleeAnimation GetRandomHitAnimation();
};
