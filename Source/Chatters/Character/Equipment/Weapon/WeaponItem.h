// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EquipmentItem.h"
#include "GameFramework/Character.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace1D.h"
#include "WeaponItem.generated.h"
	
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	Melee,
	Firearm,
	Explosion,
	Bow
};

/**
 * 
 */
UCLASS()
class CHATTERS_API UWeaponItem : public UStaticMeshEquipmentItem
{
	GENERATED_BODY()
public:
	UWeaponItem();
	~UWeaponItem();

	UPROPERTY(EditDefaultsOnly)
		int32 MinDamage = 0;

	UPROPERTY(EditDefaultsOnly)
		int32 MaxDamage = 0;

	UPROPERTY(VisibleAnywhere)
		EWeaponType Type = EWeaponType::None;

	UPROPERTY(EditDefaultsOnly)
		float TimeToPlayHitAnimation = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UBlendSpace* IdleBlendSpace = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UBlendSpace1D* HitBlendSpace = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UAnimSequence* ReloadingAnimation = nullptr;
};
