// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../WeaponItem.h"
#include "WeaponInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UWeaponInstance : public UObject
{
	GENERATED_BODY()
public:
	UWeaponInstance();
	~UWeaponInstance();

	virtual void Tick(float DeltaTime);

	virtual void Init();

	UPROPERTY(VisibleAnywhere)
		UWeaponItem* WeaponRef = nullptr;

	float TimeoutValue = 0.0f;

	bool bShouldPlayHitAnimation = false;

	float HitAnimationTime = 0.0f;

	UPROPERTY()
		ACharacter* BotOwner = nullptr;

	/** Generated new damage value between min and max */
	int32 GetDamage();
};
