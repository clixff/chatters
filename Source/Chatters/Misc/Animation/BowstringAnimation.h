// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BowstringAnimation.generated.h"

class UFirearmWeaponInstance;
class ABot;

/**
 * 
 */
UCLASS()
class CHATTERS_API UBowstringAnimation : public UAnimInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bAttachedToHand = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		FVector CenterWorldPosition;

	UPROPERTY()
		ABot* Bot = nullptr;

	UPROPERTY()
		UFirearmWeaponInstance* FirearmInstance = nullptr;

	UFUNCTION(BlueprintCallable)
		void AnimTick(float DeltaTime);
};
