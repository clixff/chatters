// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EquipmentItem.h"
#include "GameFramework/Character.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace1D.h"
#include "../../../UI/Widgets//KillFeedElement.h"
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

	UPROPERTY(EditDefaultsOnly)
		float ImpulseForce = 1000.0f;

	UPROPERTY(VisibleAnywhere)
		EWeaponType Type = EWeaponType::None;

	UPROPERTY(EditDefaultsOnly)
		float TimeToPlayHitAnimation = 0.5f;

	UPROPERTY(EditDefaultsOnly)
		bool bLoopingHitAnimation = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UBlendSpace* IdleBlendSpace = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UAnimSequence* ReloadingAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float MaxWalkSpeed = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FKillFeedIcon KillFeedIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<FMaterialSlots> TeamMaterials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UBlendSpace1D* RunBlendSpace = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UAnimSequence* HitAnimationRef;
};
