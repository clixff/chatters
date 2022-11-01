// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../Misc.h"
#include "../../Character/Bot.h"
#include "../../Character/Vehicles/Robots/Robot.h"
#include "RobotAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API URobotAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
		void AnimTick(float DeltaTime);

	ARobot* GetRobotRef();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AimingAngle = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float GunPitchRotation = 0.0f;
private:
	UPROPERTY()
		ARobot* RobotRef = nullptr;
};
