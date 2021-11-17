// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../../Character/Bot.h"
#include "BotHeadAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UBotHeadAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
		void AnimTick(float DeltaTime);

	ABot* GetBotRef();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USkeletalMeshComponent* BodyRef = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FEyesRotation EyesRotaion;
private:
	UPROPERTY()
		ABot* BotRef = nullptr;
};
