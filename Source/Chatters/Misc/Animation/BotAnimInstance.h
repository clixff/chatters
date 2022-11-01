// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../../Character/Bot.h"
#include "Animation/BlendSpace1D.h"
#include "../../Character/Equipment/Weapon/WeaponItem.h"
#include "../../Character/Equipment/Weapon/Instances/WeaponInstance.h"
#include "BotAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UBotAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
		void AnimTick(float DeltaTime);

	ABot* GetBotRef();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AimingAngle = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ECombatAction CombatAction = ECombatAction::IDLE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bShouldApplyGunAnimation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UWeaponItem* WeaponRef = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UWeaponInstance* WeaponInstance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float GunPitchRotation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bShouldPlayWeaponHitAnimation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bShouldPlayReloadingAnimation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UBlendSpace1D* RunBlendSpace = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UBlendSpace1D* DefaultRunBlendSpace = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAnimSequence* HitAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bFalling = false;
private:
	UPROPERTY()
		ABot* BotRef = nullptr;
};
